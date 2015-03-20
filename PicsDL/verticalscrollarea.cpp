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

#include <QEvent>
#include <QScrollBar>

#include "verticalscrollarea.h"

VerticalScrollArea::VerticalScrollArea(QWidget *parent) :
 QScrollArea(parent)
{
 setWidgetResizable(true);
 setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
 setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

bool VerticalScrollArea::eventFilter(QObject *o, QEvent *e)
{
 // This works because QScrollArea::setWidget installs an eventFilter on the widget
 if(o && o == widget() && e->type() == QEvent::Resize)
  setMinimumWidth(widget()->minimumSizeHint().width() + verticalScrollBar()->width());

 return QScrollArea::eventFilter(o, e);
}
