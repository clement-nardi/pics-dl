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

#ifndef INSTANCEMANAGER_H
#define INSTANCEMANAGER_H

#include <QThread>
#include <QSystemSemaphore>
#include <QSharedMemory>

class InstanceManager : public QThread
{
    Q_OBJECT
public:
    explicit InstanceManager(QObject *parent = 0);
    ~InstanceManager();
    void run();
    bool isFirstInstance;

signals:
    void applicationLaunched();

public slots:

private:
    QSystemSemaphore *sem;
    QSharedMemory *sm;
    bool isExiting;
};

#endif // INSTANCEMANAGER_H
