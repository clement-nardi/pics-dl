#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "deviceconfig.h"
#include <QGridLayout>
#include "deviceconfigview.h"
#include <QSystemTrayIcon>
#include "driveview.h"
#include <QSet>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(DeviceConfig *dc, QWidget *parent = 0);
    ~MainWindow();
    QSystemTrayIcon sysTray;

private:
    Ui::MainWindow *ui;
    DeviceConfig *dc;
    void load();
    int row;
    QGridLayout *gl;
    QSet<DriveView *> dvl;
    void showEvent(QShowEvent * event);

public slots:
    void insertDrive(QString id);
    void removeDrive(QObject * dv);
    void quit_handle();
    void show_handle();
    void sysTray_handle(QSystemTrayIcon::ActivationReason reason);

};

#endif // MAINWINDOW_H
