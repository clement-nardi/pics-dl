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

#ifdef _WIN32

#ifndef DRIVENOTOFY_WIN_H
#define DRIVENOTOFY_WIN_H

#include <shlobj.h>
#include <shlwapi.h>
#include <QWidget>

class DriveNotofy_win : public QWidget
{
    Q_OBJECT
public:
    explicit DriveNotofy_win(QWidget *parent = 0);
    ~DriveNotofy_win();
protected:
    bool nativeEvent(const QByteArray & eventType, void * message, long * result);
private:
    static const unsigned int msgShellChange = WM_USER + 1;
    unsigned long id;
    static QString getPidlPath(ITEMIDLIST* pidl);
signals:
    void driveAdded(QString);
};

#endif // DRIVENOTOFY_WIN_H

#endif // _WIN32
