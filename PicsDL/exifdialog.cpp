#include "exifdialog.h"

EXIFDialog::EXIFDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EXIFDialog)
{
    ui->setupUi(this);
}

EXIFDialog::~EXIFDialog()
{
    delete ui;
}
