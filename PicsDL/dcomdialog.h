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

#ifndef DCOMDIALOG_H
#define DCOMDIALOG_H

#include <QDialog>
#include "ui_dcomdialog.h"

namespace Ui {
class DComDialog;
}

class DComDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DComDialog(QWidget *parent = 0);
    ~DComDialog();
    Ui::DComDialog *ui;

private:
};

#endif // DCOMDIALOG_H
