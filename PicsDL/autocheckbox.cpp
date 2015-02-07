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

#include "autocheckbox.h"
#include <QStackedLayout>
#include <QTimer>

#define PB_MAX 1000
#define COUNT_DOWN 3000

autoCheckBox::autoCheckBox(QWidget *parent) :
    QWidget(parent)
{

    pb = new QProgressBar(this);
    pb->setMaximum(PB_MAX);
    pb->setValue(0);
    pb->setTextVisible(false);

    box = new QCheckBox("0-click mode",this);
    box->setStyleSheet("background:transparent");
    connect(box,SIGNAL(clicked()),this,SIGNAL(clicked()));

    QStackedLayout *sl = new QStackedLayout(this);
    sl->setStackingMode(QStackedLayout::StackAll);
    sl->addWidget(box);
    sl->addWidget(pb);

    setLayout(sl);

    countDown = new QTimer(this);
    countDown->setInterval(COUNT_DOWN);
    countDown->setSingleShot(true);
    connect(countDown,SIGNAL(timeout()),this, SLOT(treatCountDown()));

    refreshTimer = new QTimer(this);
    refreshTimer->setSingleShot(false);
    refreshTimer->setInterval(50);
    connect(refreshTimer,SIGNAL(timeout()),this, SLOT(refresh()));

}

autoCheckBox::~autoCheckBox()
{

}

void autoCheckBox::startCountDown() {
    if (box->isChecked()) {
        pb->setValue(PB_MAX);
        countDown->start();
        refreshTimer->start();
    }
}

void autoCheckBox::treatCountDown() {
    if (box->isChecked()) {
        performAction();
    }
}

void autoCheckBox::refresh() {
    int rt = countDown->remainingTime();
    if (rt > 0) {
        pb->setValue(rt*PB_MAX/COUNT_DOWN);
    } else {
        pb->setValue(0);
        refreshTimer->stop();
    }
}
