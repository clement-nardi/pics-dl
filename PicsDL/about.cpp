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
