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

#include "progressbarlabel.h"
#include <QStackedLayout>

ProgressBarLabel::ProgressBarLabel(QWidget *parent) : QWidget(parent)
{
    bar = new QProgressBar(this);
    label = new QLabel(this);
    bar->setMinimumWidth(300);
    bar->setTextVisible(false);

    QStackedLayout * layout = new QStackedLayout(this);
    layout->setStackingMode(QStackedLayout::StackAll);
    layout->addWidget(label);
    layout->addWidget(bar);
    setLayout(layout);
    setVisible(false);
}

ProgressBarLabel::~ProgressBarLabel()
{

}

