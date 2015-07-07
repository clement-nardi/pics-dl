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

#include "about.h"
#include "ui_about.h"
#include <QUrl>
#include <QDesktopServices>
#include <QDebug>
#include "globals.h"

About::About(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
    ui->picsdl_version->setText(QString("Version %1 - Build %2")
                                .arg(QCoreApplication::applicationVersion())
                                .arg(build));
    connect(ui->main_label, SIGNAL(linkActivated(QString)),this,SLOT(handleLink(QString)));
}

About::~About()
{
    delete ui;
}

void About::handleLink(QString urlStr) {
    qDebug() << "link clicked.\n";
    QUrl url(urlStr,QUrl::StrictMode);
    qDebug() << url.toString() << "\n";
    QDesktopServices::openUrl(url);
}
