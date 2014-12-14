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

void InstanceManager::run() {
    while (true) {
        sem->acquire();
        applicationLaunched();
    }
}
