#ifndef TRANSFERDIALOG_H
#define TRANSFERDIALOG_H

#include <QDialog>
#include <QTimer>
#include <QElapsedTimer>
class TransferManager;

namespace Ui {
class TransferDialog;
}

class TransferDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TransferDialog(QWidget *parent = 0);
    ~TransferDialog();
    void showProgress(TransferManager *tm_);
private slots:
    void update();
    void cancel_handle();
    void trackFile(int idx,int nb);
    void noMoreTrackFiles();

private:
    Ui::TransferDialog *ui;
    TransferManager *tm;
    QTimer progressTimer;
    QElapsedTimer statsTimer;
    qint64 lastElapsed;
    qint64 lastRead;
    qint64 lastWritten;
    qint64 maxReadThroughput;
    qint64 maxWriteThroughput;
};

#endif // TRANSFERDIALOG_H
