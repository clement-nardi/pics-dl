#include "transferdialog.h"
#include "ui_transferdialog.h"
#include "transfermanager.h"
#include "file.h"

#define GRANULARITY 100000

TransferDialog::TransferDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TransferDialog)
{
    ui->setupUi(this);
    connect(&progressTimer,SIGNAL(timeout()),this,SLOT(update()));
}

TransferDialog::~TransferDialog()
{
    delete ui;
}

static void resetProgressBar(QProgressBar *pb) {
    pb->reset();
    pb->setValue(0);
    pb->setMaximum(GRANULARITY);
    pb->setMinimum(0);
}

void TransferDialog::showProgress(TransferManager *tm_)
{
    tm = tm_;

    resetProgressBar(ui->cache_bar);
    resetProgressBar(ui->transfer_bar);
    resetProgressBar(ui->read_bar);
    resetProgressBar(ui->write_bar);
    ui->cache->setText("");
    ui->read_current->setText("");
    ui->read_max->setText("");
    ui->write_current->setText("");
    ui->write_max->setText("");
    ui->transfered_files->setText("");
    ui->transfered_data->setText("");

    lastElapsed = 0;
    lastRead = 0;
    lastWritten = 0;
    maxReadThroughput = 0;
    maxWriteThroughput = 0;

    connect(ui->cancel,SIGNAL(rejected()),this,SLOT(cancel_handle()));

    this->setWindowModality(Qt::WindowModal);
    progressTimer.setSingleShot(false);
    progressTimer.start(100);
    statsTimer.start();
    this->show();
}

void TransferDialog::update()
{
    qint64 totalElapsed = statsTimer.elapsed();
    qint64 diff = totalElapsed-lastElapsed;
    qint64 tr = tm->totalRead;
    qint64 tw = tm->totalWritten;
    qint64 ttw = tm->totalToWrite;
    qint64 ttc = tm->totalToCache;
    qint64 tc  = tr - tw;

    qint64 rt = diff?((tr-lastRead)*1000/diff):0; //Read Throughput
    qint64 wt = diff?((tw-lastWritten)*1000/diff):0; //Write Throughput
    qint64 averageThroughput = totalElapsed?(tr*1000/totalElapsed):0;

    maxReadThroughput = std::max(maxReadThroughput,rt);
    maxWriteThroughput = std::max(maxWriteThroughput,wt);

    ui->transfer_bar->setValue(ttw?tr*GRANULARITY/ttw:0);
    ui->cache_bar->setValue(ttc?tc*GRANULARITY/ttc:0);
    ui->read_bar->setValue(maxReadThroughput?rt*GRANULARITY/maxReadThroughput:0);
    ui->write_bar->setValue(maxWriteThroughput?wt*GRANULARITY/maxWriteThroughput:0);

    ui->transfered_data  ->setText(QString("%1/%2")
                                   .arg(File::size2Str(tr))
                                   .arg(File::size2Str(ttw)));
    ui->transfered_files ->setText(QString("Downloaded Files: %3/%4")
                                   .arg(tm->nbFilesTransfered)
                                   .arg(tm->nbFilesToTransfer));
    ui->read_current     ->setText(QString("Reading @ %1/s")
                                   .arg(File::size2Str(rt)));
    ui->write_current    ->setText(QString("Writing @ %1/s")
                                   .arg(File::size2Str(wt)));
    ui->read_max         ->setText(QString("max %1/s")
                                   .arg(File::size2Str(maxReadThroughput)));
    ui->write_max        ->setText(QString("max %1/s")
                                   .arg(File::size2Str(maxWriteThroughput)));
    ui->cache            ->setText(QString("Buffer: %1 / %2")
                                   .arg(File::size2Str(tc))
                                   .arg(File::size2Str(ttc)));

    lastElapsed = totalElapsed;
    lastRead = tr;
    lastWritten = tw;
}

void TransferDialog::cancel_handle() {
    progressTimer.stop();
    hide();
    tm->stopDownloads();
}
