#ifndef AUTOCHECKBOX_H
#define AUTOCHECKBOX_H

#include <QWidget>
#include <QCheckBox>
#include <QTime>
#include <QProgressBar>
#include <QTimer>

class autoCheckBox : public QWidget
{
    Q_OBJECT

public:
    explicit autoCheckBox(QWidget *parent = 0);
    ~autoCheckBox();
    QCheckBox *box;

public slots:
    void startCountDown();

signals:
    void performAction();
    void clicked();

private:
    QProgressBar *pb;
    QTimer *countDown;
    QTimer *refreshTimer;

private slots:
    void treatCountDown();
    void refresh();

};

#endif // AUTOCHECKBOX_H
