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

#include "newnameselection.h"
#include "ui_newnameselection.h"
#include <QMouseEvent>
#include <QDebug>
#include <QTextDocument>

NewNameSelection::NewNameSelection(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewNameSelection)
{
    ui->setupUi(this);

    bg = new QButtonGroup(this);

    bg->addButton(ui->radio_0, 0);
    bg->addButton(ui->radio_1, 1);
    bg->addButton(ui->radio_2, 2);
    bg->addButton(ui->radio_3, 3);
    bg->addButton(ui->radio_4, 4);

    connect(bg,SIGNAL(buttonClicked(int)),ui->destination_stack,SLOT(setCurrentIndex(int)));
    connect(this,SIGNAL(accepted()),this,SLOT(sendNewName()));

}

NewNameSelection::~NewNameSelection() {
    delete ui;
}

void NewNameSelection::mousePressEvent(QMouseEvent *event) {
    QWidget *c = childAt(event->pos());
    if (c != NULL && ui->radio_grid->indexOf(c) >= 0) {
        QWidget *w = childAt(QPoint(20,event->pos().y()));
        QRadioButton *rb = dynamic_cast<QRadioButton*> (w);
        if (rb != NULL) {
            rb->click();
        }
    }
}

void NewNameSelection::sendNewName() {
    QString richText = ((QLabel *) (ui->radio_grid->itemAtPosition(bg->checkedId(),1)->widget()))->text();
    QTextDocument td;
    td.setHtml(richText);
    emit newNameSelected(td.toPlainText());
}

