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

#ifndef WPDIODEVICE_H
#define WPDIODEVICE_H

#include <QIODevice>

class WPDIODevice : public QIODevice
{
public:
    WPDIODevice(QString IDPath_);
    ~WPDIODevice();
    bool open(OpenMode mode);
    void close();
    bool atEnd() const;
    qint64 readData(char * data, qint64 maxSize);
    qint64 writeData(const char * data, qint64 maxSize);
    QString IDPath;
    bool at_end;
};

#endif // WPDIODEVICE_H
