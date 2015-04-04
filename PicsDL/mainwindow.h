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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "config.h"
#include <QGridLayout>
#include "deviceconfigview.h"
#include <QSystemTrayIcon>
#include "driveview.h"
#include <QSet>
class DeviceModel;
class DriveNotify;
class DeviceManager;

class About;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(Config *dc, DriveNotify *dn_, DeviceManager *manager_, QWidget *parent = 0);
    ~MainWindow();
    QSystemTrayIcon sysTray;

private:
    Ui::MainWindow *ui;
    Config *dc;
    About *about;
    DeviceModel *dm;
    DeviceManager *manager;

public slots:
    void quit_handle();
    void show_handle();
    void about_handle();
    void sysTray_handle(QSystemTrayIcon::ActivationReason reason);
    void applicationLaunched();
    void setDeviceWidgets();

};

#endif // MAINWINDOW_H
