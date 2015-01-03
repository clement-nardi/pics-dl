#include "drivenotify_udev.h"

#include <libudev.h>
#include <stdio.h>
#include <QFile>
#include <QStringList>
#include <QDebug>

static void print_dev(udev_device *dev) ;


DriveNotify_udev::DriveNotify_udev(QObject *parent) :
    QThread(parent)
{
}

void DriveNotify_udev::run() {
    // inspired from http://www.signal11.us/oss/udev/
    struct udev *udev;
    struct udev_enumerate *enumerate;
    struct udev_list_entry *devices, *dev_list_entry;
    struct udev_device *dev;

    struct udev_monitor *mon;
    int fd;

    /* Create the udev object */
    udev = udev_new();
    if (!udev) {
        printf("Can't create udev\n");
        exit(1);
    }

    /* This section sets up a monitor which will report events when
       devices attached to the system change.  Events include "add",
       "remove", "change", "online", and "offline".

       This section sets up and starts the monitoring. Events are
       polled for (and delivered) later in the file.

       It is important that the monitor be set up before the call to
       udev_enumerate_scan_devices() so that events (and devices) are
       not missed.  For example, if enumeration happened first, there
       would be no event generated for a device which was attached after
       enumeration but before monitoring began.

       Note that a filter is added so that we only get events for
       "hidraw" devices. */

    /* Set up a monitor to monitor hidraw devices */
    mon = udev_monitor_new_from_netlink(udev, "udev");
    //udev_monitor_filter_add_match_subsystem_devtype(mon, "hidraw", NULL);
    udev_monitor_enable_receiving(mon);
    /* Get the file descriptor (fd) for the monitor.
       This fd will get passed to select() */
    fd = udev_monitor_get_fd(mon);


    /* Create a list of the devices in the 'hidraw' subsystem. */
    enumerate = udev_enumerate_new(udev);
    //udev_enumerate_add_match_subsystem(enumerate, "hidraw");
    udev_enumerate_scan_devices(enumerate);
    devices = udev_enumerate_get_list_entry(enumerate);
    /* For each item enumerated, print out its information.
       udev_list_entry_foreach is a macro which expands to
       a loop. The loop will be executed for each member in
       devices, setting dev_list_entry to a list entry
       which contains the device's path in /sys. */
    udev_list_entry_foreach(dev_list_entry, devices) {
        const char *path;

        /* Get the filename of the /sys entry for the device
           and create a udev_device object (dev) representing it */
        path = udev_list_entry_get_name(dev_list_entry);
        dev = udev_device_new_from_syspath(udev, path);

        /* usb_device_get_devnode() returns the path to the device node
           itself in /dev. */
        printf("Device Node Path: %s\n", udev_device_get_devnode(dev));

        /* The device pointed to by dev contains information about
           the hidraw device. In order to get information about the
           USB device, get the parent device with the
           subsystem/devtype pair of "usb"/"usb_device". This will
           be several levels up the tree, but the function will find
           it.*/
        dev = udev_device_get_parent_with_subsystem_devtype(
               dev,
               "usb",
               "usb_device");
        if (!dev) {
            printf("Unable to find parent usb device.");
            exit(1);
        }

        /* From here, we can call get_sysattr_value() for each file
           in the device's /sys entry. The strings passed into these
           functions (idProduct, idVendor, serial, etc.) correspond
           directly to the files in the /sys directory which
           represents the USB device. Note that USB strings are
           Unicode, UCS2 encoded, but the strings returned from
           udev_device_get_sysattr_value() are UTF-8 encoded. */
        printf("  VID/PID: %s %s\n",
                udev_device_get_sysattr_value(dev,"idVendor"),
                udev_device_get_sysattr_value(dev, "idProduct"));
        printf("  %s\n  %s\n",
                udev_device_get_sysattr_value(dev,"manufacturer"),
                udev_device_get_sysattr_value(dev,"product"));
        printf("  serial: %s\n",
                 udev_device_get_sysattr_value(dev, "serial"));
        udev_device_unref(dev);
    }
    /* Free the enumerator object */
    udev_enumerate_unref(enumerate);

    /* Begin polling for udev events. Events occur when devices
       attached to the system are added, removed, or change state.
       udev_monitor_receive_device() will return a device
       object representing the device which changed and what type of
       change occured.

       The select() system call is used to ensure that the call to
       udev_monitor_receive_device() will not block.

       The monitor was set up earler in this file, and monitoring is
       already underway.

       This section will run continuously, calling usleep() at the end
       of each pass. This is to demonstrate how to use a udev_monitor
       in a non-blocking way. */
    while (1) {
        /* Set up the call to select(). In this case, select() will
           only operate on a single file descriptor, the one
           associated with our udev_monitor. Note that the timeval
           object is set to 0, which will cause select() to not
           block. */
        fd_set fds;
        struct timeval tv;
        int ret;

        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        ret = select(fd+1, &fds, NULL, NULL, &tv);

        /* Check if our file descriptor has received data. */
        if (ret > 0 && FD_ISSET(fd, &fds)) {
            printf("\nselect() says there should be data\n");

            /* Make the call to receive the device.
               select() ensured that this will not block. */
            dev = udev_monitor_receive_device(mon);
            if (dev) {
                printf("%s Device\n", udev_device_get_action(dev));
                if (udev_device_get_devnode(dev)) {
                    //print_dev(dev);
                    printf("   devnode: %s\n", udev_device_get_devnode(dev));

                    QFile mountList("/proc/mounts");
                    mountList.open(QIODevice::ReadOnly);
                    while (1) {
                        QByteArray line = mountList.readLine();
                        if (line.isEmpty()) break;
                        QStringList mountInfo = QString(line).split(" ");
                        if (mountInfo.size()>=2 && mountInfo.at(0) == udev_device_get_devnode(dev)) {
                            qDebug() << mountInfo.at(0) << mountInfo.at(1);
                        }
                    }
                    mountList.close();
                } else {
                    printf("   without devnode\n");
                }

                //emit driveAdded(QString("%1 %2 %3").arg(udev_device_get_devnode(dev)).arg(udev_device_get_subsystem(dev)).arg(udev_device_get_devtype(dev)));

                udev_device_unref(dev);
            }
            else {
                printf("No Device from receive_device(). An error occured.\n");
            }
        } else {
            usleep(200*1000);
        }
        //printf(".");
        fflush(stdout);
    }


    udev_unref(udev);

    return ;
}



static void print_dev(udev_device *dev) {
    printf("   devpath:                 %s\n", udev_device_get_devpath(dev));
    printf("   subsystem:               %s\n", udev_device_get_subsystem(dev));
    printf("   devtype:                 %s\n", udev_device_get_devtype(dev));
    printf("   syspath:                 %s\n", udev_device_get_syspath(dev));
    printf("   sysname:                 %s\n", udev_device_get_sysname(dev));
    printf("   sysnum:                  %s\n", udev_device_get_sysnum(dev));
    printf("   devnode:                 %s\n", udev_device_get_devnode(dev));
    printf("   is_initialized:          %d\n", udev_device_get_is_initialized(dev));
    printf("   driver:                  %s\n", udev_device_get_driver(dev));
    printf("   action:                  %s\n", udev_device_get_action(dev));
    printf("   seqnum:                  %d\n", udev_device_get_seqnum(dev));
    printf("   usec_since_initialized:  %d\n", udev_device_get_usec_since_initialized(dev));

    int idx;
    struct udev_list_entry * el;

    idx = 0;
    el = udev_device_get_properties_list_entry(dev);
    while (el != NULL) {
    printf("   property %d:             %s=%s\n", ++idx, udev_list_entry_get_name(el),
                                                         udev_list_entry_get_value(el));
        el = udev_list_entry_get_next(el);
    }

    idx = 0;
    el = udev_device_get_devlinks_list_entry(dev);
    while (el != NULL) {
    printf("   devlink %d:              %s=%s\n", ++idx, udev_list_entry_get_name(el),
                                                         udev_list_entry_get_value(el));
        el = udev_list_entry_get_next(el);
    }

    idx = 0;
    el = udev_device_get_tags_list_entry(dev);
    while (el != NULL) {
    printf("   tag %d:                  %s=%s\n", ++idx, udev_list_entry_get_name(el),
                                                         udev_list_entry_get_value(el));
        el = udev_list_entry_get_next(el);
    }

    idx = 0;
    el = udev_device_get_sysattr_list_entry(dev);
    while (el != NULL) {
    printf("   sysattr %d:              %s=%s\n", ++idx, udev_list_entry_get_name(el),
                                                         udev_list_entry_get_value(el));
        el = udev_list_entry_get_next(el);
    }
}
