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

#include "instancemanager.h"
#include <QCoreApplication>

InstanceManager::InstanceManager(QObject *parent) :
    QThread(parent) {
    sm = new QSharedMemory(QCoreApplication::applicationName() + "_MultiInstance_SharedMemory");
    sem = new QSystemSemaphore(QCoreApplication::applicationName() + "_MultiInstance_Semaphore", 0, QSystemSemaphore::Open);
    if (!sm->create(1)) {
        isFirstInstance = false;
        sem->release(1);
        exit(0);
    } else {
        isFirstInstance = true;
        this->start();
    }
}

InstanceManager::~InstanceManager() {
    delete sm;
}

void InstanceManager::run() {
    while (true) {
        sem->acquire();
        emit applicationLaunched();
    }
}
