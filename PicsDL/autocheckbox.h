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

#ifndef AUTOCHECKBOX_H
#define AUTOCHECKBOX_H

#include <QWidget>
#include <QCheckBox>
#include <QTime>
#include <QProgressBar>
#include <QTimer>

class autoCheckBox : public QWidget
{
    Q_OBJECT

public:
    explicit autoCheckBox(QWidget *parent = 0);
    ~autoCheckBox();
    QCheckBox *box;

public slots:
    void startCountDown();

signals:
    void performAction();
    void clicked();

private:
    QProgressBar *pb;
    QTimer *countDown;
    QTimer *refreshTimer;

private slots:
    void treatCountDown();
    void refresh();

};

#endif // AUTOCHECKBOX_H
