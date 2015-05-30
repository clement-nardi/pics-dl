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
