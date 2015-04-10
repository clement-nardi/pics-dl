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


MainWindow::MainWindow(Config *dc_, DriveNotify *dn_, DeviceManager *manager_,  QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    dc = dc_;
    manager = manager_;
    ui->setupUi(this);
    about =  new About();

    QMenu *menu = new QMenu;
    menu->addAction(ui->actionShow);
    menu->addAction(ui->actionAbout);
    menu->addAction(ui->actionQuit);
    menu->setDefaultAction(ui->actionShow);
    sysTray.setContextMenu(menu);
    sysTray.setIcon(QIcon(":/icons/picsDL"));
    sysTray.show();

    connect(&sysTray,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(sysTray_handle(QSystemTrayIcon::ActivationReason)));
    connect(ui->actionQuit, SIGNAL(triggered()),this,SLOT(quit_handle()));
    connect(ui->actionShow, SIGNAL(triggered()),this,SLOT(show_handle()));
    connect(ui->actionAbout, SIGNAL(triggered()),this,SLOT(about_handle()));

    dm = new DeviceModel(dc,dn_);
    ui->deviceTable->setModel(dm);
    setDeviceWidgets();
    ui->deviceTable->resizeColumnsToContents();
    connect(dm,SIGNAL(modelReset()),this,SLOT(setDeviceWidgets()));
    dc->LoadWindowGeometry("MainWindow",this);
}

void MainWindow::setDeviceWidgets() {
    for (int i = 0; i < dm->deviceList.size(); i++) {
        DriveView *dv = dm->deviceList.at(i);
        ui->deviceTable->setIndexWidget(dm->index(i,COL_MANAGE),dv->managedBox_centered);
        ui->deviceTable->setIndexWidget(dm->index(i,COL_EDIT),dv->editButton);
        ui->deviceTable->setIndexWidget(dm->index(i,COL_LAUNCH),dv->launchButton);
        ui->deviceTable->setIndexWidget(dm->index(i,COL_REMOVE),dv->removeButton);
        connect(dv,SIGNAL(launchTransfer(QString,QString,QString,qint64,qint64,bool)),
                manager,SLOT(treatDrive(QString,QString,QString,qint64,qint64,bool)));
    }
}

void MainWindow::sysTray_handle(QSystemTrayIcon::ActivationReason reason) {
    switch(reason) {
    case QSystemTrayIcon::DoubleClick:
    case QSystemTrayIcon::Trigger:
        sysTray.contextMenu()->defaultAction()->trigger();
        break;
    }
}

void MainWindow::show_handle() {
    show();
    if (dm->deviceList.size() == 0) {
        QMessageBox msgBox;
        msgBox.setText(QString(tr("No known device.")));
        msgBox.setInformativeText(tr("PicsDL does not know any of your devices! Please plug a device (smartphone, memory card, camera, USB key) in order to start downloading the pictures it contains."));
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setStandardButtons(QMessageBox::Ok);
        int ret = msgBox.exec();
    }
}

void MainWindow::about_handle() {
    about->show();
}

void MainWindow::applicationLaunched() {
    qDebug() << "applicationLaunched()";
    sysTray.showMessage("PicsDL is running in the background",
                        "Please plug your device (smartphone, memory card, camera, USB key)",
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


MainWindow::~MainWindow()
{
    qDebug() << "delete MainWindow";
    dc->SaveWindowGeometry(this,"MainWindow");
    delete ui;
    about->deleteLater();
}
