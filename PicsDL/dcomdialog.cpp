#include "dcomdialog.h"

DComDialog::DComDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DComDialog)
{
    ui->setupUi(this);
}

DComDialog::~DComDialog()
{
    delete ui;
}
