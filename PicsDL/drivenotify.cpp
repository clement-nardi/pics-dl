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

#include "drivenotify.h"
#include <QWidget>
#include <QDebug>

#ifdef _WIN32
#include "drivenotify_win.h"
#else
#include "drivenotify_unix.h"
#endif

DriveNotify::DriveNotify(QObject *parent) :
    QObject(parent)
{

#ifdef _WIN32
    DriveNotofy_win *dnw = new DriveNotofy_win();
    connect(dnw,SIGNAL(driveAdded(QString)),this,SIGNAL(driveAdded(QString)));
#else
    DriveNotify_unix *dnu = new DriveNotify_unix();
    connect(dnu,SIGNAL(driveAdded(QString)),this,SIGNAL(driveAdded(QString)),Qt::QueuedConnection);
#endif

}
