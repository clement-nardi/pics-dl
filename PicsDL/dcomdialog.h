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
