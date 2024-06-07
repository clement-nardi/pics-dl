/**
 * Copyright 2014-2015 Clément Nardi
 *
 * This file is part of PicsDL.
 *
 * PicsDL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PicsDL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PicsDL.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

#include "transferdialog.h"
#include "ui_transferdialog.h"
#include "transfermanager.h"
#include "file.h"
#include "geotagger.h"

#define GRANULARITY 100000

TransferDialog::TransferDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TransferDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Drawer
                   | Qt::WindowTitleHint
                   | Qt::CustomizeWindowHint);
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

    ui->track_bar->setVisible(false);
    ui->track_label->setVisible(false);
    if (tm->geotagger != NULL) {
        connect(tm->geotagger,SIGNAL(loadingTrackFile(int,int)),this,SLOT(trackFile(int,int)));
        connect(tm->geotagger,SIGNAL(loadingTrackFilesFinished()),this,SLOT(noMoreTrackFiles()));
    }
    adjustSize();
    this->show();
}

void TransferDialog::update()
{
    qint64 totalElapsed = statsTimer.elapsed();
    qint64 diff = totalElapsed-lastElapsed;
    qint64 toralRead = tm->totalRead;
    qint64 tw = tm->totalWritten;
    qint64 ttw = tm->totalToWrite;
    qint64 ttc = tm->totalToCache;
    qint64 tc  = toralRead - tw;

    qint64 rt = diff?((toralRead-lastRead)*1000/diff):0; //Read Throughput
    qint64 wt = diff?((tw-lastWritten)*1000/diff):0; //Write Throughput
    //qint64 averageThroughput = totalElapsed?(tr*1000/totalElapsed):0;

    maxReadThroughput = std::max(maxReadThroughput,rt);
    maxWriteThroughput = std::max(maxWriteThroughput,wt);

    ui->transfer_bar->setValue(ttw?toralRead*GRANULARITY/ttw:0);
    ui->cache_bar->setValue(ttc?tc*GRANULARITY/ttc:0);
    ui->read_bar->setValue(maxReadThroughput?rt*GRANULARITY/maxReadThroughput:0);
    ui->write_bar->setValue(maxWriteThroughput?wt*GRANULARITY/maxWriteThroughput:0);

    ui->transfered_data  ->setText(QString("%1/%2")
                                   .arg(File::size2Str(toralRead))
                                   .arg(File::size2Str(ttw)));
    ui->transfered_files ->setText(QString(tr("Downloaded Files: %3/%4"))
                                   .arg(tm->nbFilesTransfered)
                                   .arg(tm->nbFilesToTransfer));
    ui->read_current     ->setText(QString(tr("Reading @ %1/s"))
                                   .arg(File::size2Str(rt)));
    ui->write_current    ->setText(QString(tr("Writing @ %1/s"))
                                   .arg(File::size2Str(wt)));
    ui->read_max         ->setText(QString(tr("max %1/s"))
                                   .arg(File::size2Str(maxReadThroughput)));
    ui->write_max        ->setText(QString(tr("max %1/s"))
                                   .arg(File::size2Str(maxWriteThroughput)));
    ui->cache            ->setText(QString(tr("Buffer: %1 / %2"))
                                   .arg(File::size2Str(tc))
                                   .arg(File::size2Str(ttc)));

    lastElapsed = totalElapsed;
    lastRead = toralRead;
    lastWritten = tw;
}

void TransferDialog::cancel_handle() {
    progressTimer.stop();
    hide();
    tm->stopDownloads();
}


void TransferDialog::trackFile(int idx,int nb){
    ui->track_bar->setMaximum(nb);
    ui->track_bar->setValue(idx);
    ui->track_label->setText(QString(tr("Loading track file %1/%2"))
                             .arg(idx)
                             .arg(nb));
    ui->track_bar->setVisible(true);
    ui->track_label->setVisible(true);
}

void TransferDialog::noMoreTrackFiles(){
    ui->track_bar->setVisible(false);
    ui->track_label->setVisible(false);
    setMaximumHeight(1);
    adjustSize();
    setMaximumHeight(400);
}
