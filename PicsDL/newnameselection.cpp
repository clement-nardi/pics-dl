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

