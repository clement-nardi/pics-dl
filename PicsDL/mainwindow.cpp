/**
 * Copyright 2014-2015 Clément Nardi
 *
 * This file is part of PicsDL.
 *
 * PicsDL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PicsDL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PicsDL.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGridLayout>
#include <QJsonObject>
#include <QLabel>
#include "driveview.h"
#include <QMessageBox>
#include "verticalscrollarea.h"
#include "about.h"
#include "devicemodel.h"
#include "devicemanager.h"
#include <QDebug>
#include <QStandardPaths>
#include <QFileDialog>
#include <QFileInfoList>
#include <QTranslator>


MainWindow::MainWindow(Config *dc_, DriveNotify *dn_, DeviceManager *manager_,  QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    dc = dc_;
    manager = manager_;
    ui->setupUi(this);
    about =  new About();
    currentTranslator = new QTranslator(this);

    QMenu *menu = new QMenu;
    menu->addAction(ui->actionShow);
    menu->addAction(ui->actionAbout);
    menu->addAction(ui->actionQuit);
    menu->setDefaultAction(ui->actionShow);
    sysTray.setContextMenu(menu);
    sysTray.setIcon(QIcon(":/icons/picsDL"));
    sysTray.show();
#ifdef __APPLE__
    menu->setAsDockMenu();
#endif

    QFileInfoList languageFiles = QDir(QCoreApplication::applicationDirPath()).entryInfoList();
    QRegExp languageFilePattern = QRegExp("picsdl_(.*)\\.qm");
    languageActionGroup = new QActionGroup(this);
    connect(languageActionGroup,SIGNAL(triggered(QAction*)),this,SLOT(language_handle(QAction*)));
    addLanguage("en");
    for (int i = 0; i< languageFiles.size(); i++) {
        QFileInfo languageFile = languageFiles.at(i);
        //qDebug() << "listing content of executable directory: " << languageFile.absoluteFilePath();
        if (languageFilePattern.indexIn(languageFile.fileName()) >= 0) {
            qDebug() << "language file found: " << languageFile.absoluteFilePath();
            QString languageCode = languageFilePattern.cap(1);
            if (languageCode != "blank_translation") {
                addLanguage(languageCode);
            }
        }
    }

    connect(&sysTray,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(sysTray_handle(QSystemTrayIcon::ActivationReason)));
    connect(ui->actionQuit, SIGNAL(triggered()),this,SLOT(quit_handle()));
    connect(ui->actionShow, SIGNAL(triggered()),this,SLOT(show_handle()));
    connect(ui->actionAbout, SIGNAL(triggered()),this,SLOT(about_handle()));
    connect(ui->actionDownload_pictures_from_Folder, SIGNAL(triggered()),this,SLOT(dl_from_folder_handle()));
    connect(ui->actionReorganize_a_folder, SIGNAL(triggered()),this,SLOT(reorganize_handle()));
    connect(ui->actionGeotag_some_pictures, SIGNAL(triggered()),this,SLOT(geotag_handle()));

    dm = new DeviceModel(dc,dn_);
    ui->deviceTable->horizontalHeader()->setSortIndicator(COL_LAUNCH,Qt::DescendingOrder);
    ui->deviceTable->setModel(dm);
    setDeviceWidgets();
    ui->deviceTable->resizeColumnsToContents();
    connect(dm,SIGNAL(modelReset()),this,SLOT(setDeviceWidgets()));
    dc->LoadWindowGeometry("MainWindow",this);
}

void MainWindow::addLanguage(QString languageCode) {
    qDebug() << "addLanguage: " << languageCode;
    QAction * action = ui->menuLanguage->addAction(QIcon(QString(":/icons/resources/flag_%1.png").arg(languageCode)),
                                                   languageCode);
    action->setActionGroup(languageActionGroup);
    action->setCheckable(true);
    if (languageCode == dc->gui_params["language"].toString()) {
        action->setChecked(true);
        language_handle(action);
    }
}

void MainWindow::language_handle(QAction *action) {
    qDebug() << QString("language_handle(%1)").arg(action->text());

    QApplication::removeTranslator(currentTranslator);

    if (currentTranslator->load("picsdl_" + action->text(),QCoreApplication::applicationDirPath())) {
        qDebug() << "language file was successfully loaded!";
        QApplication::installTranslator(currentTranslator);
        QLocale::setDefault(QLocale(action->text()));
    } else {
        qDebug() << "language file was NOT successfully loaded... -> back to English + system's locale";
        QLocale::setDefault(QLocale::system());
    }
    ui->retranslateUi(this);
    delete about;
    about = new About();

    dc->gui_params.insert("language",action->text());
    dc->saveGUIParams();
}


void MainWindow::setDeviceWidgets() {
    for (int i = 0; i < dm->deviceList.size(); i++) {
        DriveView *dv = dm->deviceList.at(i);
        ui->deviceTable->setIndexWidget(dm->index(i,COL_MANAGE),dv->managedBox_centered);
        ui->deviceTable->setIndexWidget(dm->index(i,COL_EDIT),dv->editButton);
        ui->deviceTable->setIndexWidget(dm->index(i,COL_LAUNCH),dv->launchButton);
        ui->deviceTable->setIndexWidget(dm->index(i,COL_REMOVE),dv->removeButton);
        connect(dv,SIGNAL(launchTransfer(QString,bool)),
                manager,SLOT(treatDrive(QString,bool)));
    }
}

/*
bool MainWindow::event(QEvent * event){
    //qDebug() << "event" << event;
    return QMainWindow::event(event);
}

bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, long *result){
    MSG* msg = reinterpret_cast<MSG*> (message);
    //qDebug() << "MainWindow::nativeEvent " << msg->message << msg->wParam << msg->lParam << msg->time << msg->pt.x << msg->pt.y ;
    if (msg->message == WM_QUIT ||
            msg->message == WM_CLOSE ||
            msg->message == WM_QUERYENDSESSION ||
            msg->message == WM_ENDSESSION) {
        QCoreApplication::exit(0);
    }
    return QMainWindow::nativeEvent(eventType, message, result);
}
*/

void MainWindow::sysTray_handle(QSystemTrayIcon::ActivationReason reason) {
    switch(reason) {
    case QSystemTrayIcon::DoubleClick:
    case QSystemTrayIcon::Trigger:
        sysTray.contextMenu()->defaultAction()->trigger();
        break;
    default:
        break;
    }
}

void MainWindow::show_handle() {
    static bool welcomeIsShown = false; /* in case the function is called twice */
    show();
    if (dm->deviceList.size() == 0 && !welcomeIsShown) {
        QMessageBox msgBox;
        msgBox.setText(QString(tr("No known device.")));
        msgBox.setInformativeText(tr("PicsDL does not know any of your devices! Please plug a device (smartphone, memory card, camera, USB key) in order to start downloading the pictures it contains."));
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setStandardButtons(QMessageBox::Ok);
        welcomeIsShown = true;
        msgBox.exec();
        welcomeIsShown = false;
    }
}

void MainWindow::about_handle() {
    about->show();
}

void MainWindow::applicationLaunched() {
    qDebug() << "applicationLaunched()";
    sysTray.showMessage(tr("PicsDL is running in the background"),
                        tr("Please plug your device (smartphone, memory card, camera, USB key)"),
                        QSystemTrayIcon::Information);
}

void MainWindow::quit_handle() {
    QMessageBox msgBox;
    msgBox.setText(QString(tr("Are you sure you want to exit PicsDL?")));
    msgBox.setInformativeText(tr("PicsDL needs to run in the background in order to detect when you plug a camera or memory card to your computer."));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();

    if (ret==QMessageBox::Yes) {
        QApplication::exit(0);
    }
}

void MainWindow::dl_from_folder_handle() {
    QString dir = QFileDialog::getExistingDirectory(this,tr("Choose the directory where the pictures are located"),
                                           QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0));
    if (dir.size()>0) {
        QJsonObject obj = dc->devices[dir].toObject();
        obj[CONFIG_DISPLAYNAME]     = dir;
        obj[CONFIG_PATH]            = dir;
        obj[CONFIG_DEVICETYPE]      = "Folder";
        obj[CONFIG_ISMANAGED]       = false;
        obj[CONFIG_ALLOWEXIF]       = true;
        obj[CONFIG_USEEXIFDATE]     = true;
        dc->devices.insert(dir,obj);
        dc->deviceFieldChanged(dir);
        manager->treatDrive(dir,true);
    }
}

void MainWindow::reorganize_handle() {
    QString dir = QFileDialog::getExistingDirectory(this,tr("Choose the directory to reorganize"),
                                           QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0));
    if (dir.size()>0) {
        QJsonObject obj = dc->devices[dir].toObject();
        obj[CONFIG_DISPLAYNAME]     = dir;
        obj[CONFIG_PATH]            = dir;
        obj[CONFIG_DEVICETYPE]      = "Folder";
        obj[CONFIG_ISMANAGED]       = false;
        obj[CONFIG_FILESTODOWNLOAD] = "All";
        obj[CONFIG_DOWNLOADTO]      = dir;
        obj[CONFIG_MOVEFILES]       = true;
        obj[CONFIG_ALLOWEXIF]       = true;
        obj[CONFIG_USEEXIFDATE]     = true;
        dc->devices.insert(dir,obj);
        dc->deviceFieldChanged(dir);
        manager->treatDrive(dir,true);
    }
}
void MainWindow::geotag_handle() {
    QString dir = QFileDialog::getExistingDirectory(this,tr("Choose the directory containing the files to geotag"),
                                           QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0));
    if (dir.size()>0) {
        QJsonObject obj = dc->devices[dir].toObject();
        obj[CONFIG_DISPLAYNAME]     = dir;
        obj[CONFIG_PATH]            = dir;
        obj[CONFIG_DEVICETYPE]      = "Folder";
        obj[CONFIG_ISMANAGED]       = false;
        obj[CONFIG_FILESTODOWNLOAD] = "All";
        obj[CONFIG_DOWNLOADTO]      = dir;
        obj[CONFIG_MOVEFILES]       = true;
        obj[CONFIG_OVERWRITEFILES]  = true;
        obj[CONFIG_NEWNAME]         = "oFolder1-/oName";
        obj[CONFIG_GEOTAG]          = true;
        obj[CONFIG_ALLOWEXIF]       = true;
        obj[CONFIG_USEEXIFDATE]     = true;
        dc->devices.insert(dir,obj);
        dc->deviceFieldChanged(dir);
        manager->treatDrive(dir,true);
    }
}

MainWindow::~MainWindow()
{
    qDebug() << "delete MainWindow";
    dc->saveWindowGeometry(this,"MainWindow");
    delete ui;
    about->deleteLater();
}
