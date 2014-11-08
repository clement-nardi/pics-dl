#ifndef EXIFDIALOG_H
#define EXIFDIALOG_H

#include <QDialog>
#include "ui_exifdialog.h"

namespace Ui {
class EXIFDialog;
}

class EXIFDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EXIFDialog(QWidget *parent = 0);
    ~EXIFDialog();
    Ui::EXIFDialog *ui;

private:
};

#endif // EXIFDIALOG_H
