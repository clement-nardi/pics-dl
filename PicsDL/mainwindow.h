/**
 * Copyright 2014 Clément Nardi
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
    void applicationLaunched();

};

#endif // MAINWINDOW_H
