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

#ifndef NEWNAMESELECTION_H
#define NEWNAMESELECTION_H

#include <QDialog>
#include <QButtonGroup>

namespace Ui {
class NewNameSelection;
}

class NewNameSelection : public QDialog
{
    Q_OBJECT

public:
    explicit NewNameSelection(QWidget *parent = 0);
    ~NewNameSelection();

private:
    Ui::NewNameSelection *ui;
    QButtonGroup *bg;
    void mousePressEvent(QMouseEvent * event);
signals:
    void newNameSelected(QString);
private slots:
    void sendNewName();
};

#endif // NEWNAMESELECTION_H
