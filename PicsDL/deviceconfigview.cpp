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

#include "deviceconfigview.h"
#include "ui_deviceconfigview.h"
#include <QStandardPaths>
#include <QFileDialog>
#include <QTextEdit>
#include <QDebug>
#include <QTimer>
#include <QScreen>
#include "transfermanager.h"
#include "downloadmodel.h"
#include "deviceconfig.h"
#include "transferdialog.h"

DeviceConfigView::DeviceConfigView(DeviceConfig *dc_, QString id, bool editMode_, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeviceConfigView)
{
    dpm = NULL;
    tm = NULL;
    dc = dc_;
    editMode = editMode_;
    pd = new QProgressDialog(this);
    td = new TransferDialog(this);
    ui->setupUi(this);

    connect(ui->openButton, SIGNAL(clicked()),this, SLOT(chooseDLTo()));
    connect(ui->trackOpenButton, SIGNAL(clicked()),this, SLOT(chooseTrackFolder()));
    ui->tableView->setWordWrap(false);

    setAttribute(Qt::WA_DeleteOnClose, true);
    setAttribute(Qt::WA_QuitOnClose, false);
    FillWithConfig(id);

    if (!editMode) {
        dpm = new DownloadModel(dc, pd, editMode);
        connect(dpm, SIGNAL(itemOfInterest(QModelIndex)), this, SLOT(makeVisible(QModelIndex)));
        connect(dpm, SIGNAL(reloaded()), ui->tableView, SLOT(resizeColumnsToContents()));
        connect(dpm, SIGNAL(reloaded()), ui->tableView, SLOT(resizeRowsToContents()));
        connect(ui->FillCommentsButton,SIGNAL(clicked()), dpm, SLOT(getAllCom()));
        connect(dpm, SIGNAL(EXIFLoadCanceled(bool)), ui->allowEXIFBox, SLOT(setChecked(bool)));
        connect(ui->seeAvTagsButton, SIGNAL(clicked()), this, SLOT(showEXIFTags()));

        ui->tableView->setModel(dpm);
        dpm->loadPreview(id);
        tm = new TransferManager(this,dpm);
        //ui->tableView->setItemDelegate(dpm->getItemDelegate());
        //ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

        connect(ui->automation, SIGNAL(performAction()),this,SLOT(go()));
    }
    //setCornerWidget(new QPushButton("Go",this),Qt::BottomRightCorner);

    updateStatusText();
    showMaximized();

    /* automation stuff */
    ui->automation->startCountDown();

}

DeviceConfigView::~DeviceConfigView()
{
    qDebug() << "delete DeviceConfigView";
    delete ui;
    delete dpm;
    delete pd;
}


void DeviceConfigView::showEXIFTags(){
    if (ui->tableView->selectionModel()->currentIndex().isValid()) {
        if (dpm != NULL) { /* not in edit mode */
            dpm->showEXIFTags(ui->tableView->selectionModel()->currentIndex().row());
        }
    }
}

void DeviceConfigView::makeVisible(QModelIndex mi) {
    ui->tableView->scrollTo(mi);
}


void DeviceConfigView::showEvent(QShowEvent * event) {
    qDebug() << "showEvent";
    //automation(currentId());
    QWidget::showEvent(event);
}

void DeviceConfigView::FillWithConfig(QString id_) {
    qDebug() << "FillWithConfig";
    id = id_;
    QJsonObject obj = dc->conf[id].toObject();
    QString displayName = obj["displayName"].toString();
    QString path = obj["path"].toString();
    QString title = QCoreApplication::applicationName() + " v" + QCoreApplication::applicationVersion()
                    + " - " + displayName;
    if ( displayName != path) {
        title += " - " + path;
    }
    this->setWindowTitle(title);

    /* Download part */
    if (obj["FilesToDownLoad"].isNull() || obj["FilesToDownLoad"].isUndefined()) {
        obj.insert("FilesToDownLoad",QJsonValue(QString("New")));
    }
    if (obj["FilesToDownLoad"].toString() == "All") {
        ui->allRadio->setChecked(true);
    } else {
        ui->newRadio->setChecked(true);
    }

    if (obj["FilterType"].isNull() || obj["FilterType"].isUndefined()) {
        if (obj["IDPath"].toString().startsWith("WPD")) {
            obj.insert("FilterType",QJsonValue(2));
            obj.insert("Filter",QJsonValue(QString("DCIM;Camera;Auto;*ANDRO;user;media;*iPhone*;*APPLE;Pictures;Screenshots;Download;WhatsApp*;Media")));
        } else {
            obj.insert("FilterType",QJsonValue(0));
            obj.insert("Filter",QJsonValue(QString("")));
        }
    }
    ui->filterType->setCurrentIndex(obj["FilterType"].toInt());
    ui->filter->setText(obj["Filter"].toString());
    ui->filter->setEnabled(ui->filterType->currentIndex() != 0);

    /* Organize part */
    if (obj["DownloadTo"].isNull() || obj["DownloadTo"].isUndefined()) {
        obj.insert("DownloadTo",QJsonValue(QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0)));
    }
    ui->DLToLine->setText(obj["DownloadTo"].toString());
    if (obj["newName"].isNull() || obj["newName"].isUndefined()) {
        obj.insert("newName",QJsonValue(QString("yyyy-MM/yyyy-MM-dd/yyyy-MM-dd_HH-mm-ss_oName")));
    }
    ui->newNameEdit->setText(obj["newName"].toString());
    if (obj["AllowEXIF"].isNull() || obj["AllowEXIF"].isUndefined()) {
        obj.insert("AllowEXIF",QJsonValue(false));
    }
    ui->allowEXIFBox->setChecked(obj["AllowEXIF"].toBool());
    if (obj["UseEXIFDate"].isNull() || obj["UseEXIFDate"].isUndefined()) {
        obj.insert("UseEXIFDate",QJsonValue(true));
    }
    ui->UseEXIFDateBox->setChecked(obj["UseEXIFDate"].toBool());
    /* Automation */
    if (obj["automation"].isNull() || obj["automation"].isUndefined()) {
        obj.insert("automation",QJsonValue(false));
    }
    ui->automation->box->setChecked(obj["automation"].toBool());

    /* Geo Tagging */
    if (obj["GeoTag"].isNull() || obj["GeoTag"].isUndefined()) {
        obj.insert("GeoTag",QJsonValue(false));
    }
    ui->GeoTagBox->setChecked(obj["GeoTag"].toBool());
    if (obj["GeoTagMode"].isNull() || obj["GeoTagMode"].isUndefined()) {
        obj.insert("GeoTagMode",QJsonValue(QString("Track Folder")));
    }
    if (obj["GeoTagMode"].toString() == "OpenPaths.cc") {
        ui->OpenPathsRadio->setChecked(true);
    } else {
        ui->TrackFolderRadio->setChecked(true);
    }
    if (obj["TrackFolder"].isNull() || obj["TrackFolder"].isUndefined()) {
        obj.insert("TrackFolder",QJsonValue(QString("")));
    }
    ui->TrackFolderLine->setText(obj["TrackFolder"].toString());
    if (obj["OPAccessKey"].isNull() || obj["OPAccessKey"].isUndefined()) {
        obj.insert("OPAccessKey",QJsonValue(QString("")));
    }
    ui->OPAccessKeyLine->setText(obj["OPAccessKey"].toString());
    if (obj["OPSecretKey"].isNull() || obj["OPSecretKey"].isUndefined()) {
        obj.insert("OPSecretKey",QJsonValue(QString("")));
    }
    ui->OPSecretKeyLine->setText(obj["OPSecretKey"].toString());


    dc->conf[id] = obj;
    dc->saveConfig();

    //ui->tableView->resizeRowsToContents();

    updateButton();
}


void DeviceConfigView::updateButton(){
    qDebug() << "updateButton";
    if (ui->newNameEdit->text().contains("dCom") ||
        ui->newNameEdit->text().contains("sCom") ) {
        ui->FillCommentsButton->setEnabled(true);
    } else {
        ui->FillCommentsButton->setDisabled(true);
    }
    bool exifAllowed = ui->allowEXIFBox->isChecked();
    ui->UseEXIFDateBox->setEnabled(exifAllowed);
    ui->seeAvTagsButton->setEnabled(exifAllowed);
}

void DeviceConfigView::CopyToConfig() {
    qDebug() << "CopyToConfig";
    QJsonObject obj = dc->conf[id].toObject();
    bool reloadNeeded = false;

    /* Download part */
    if ((obj["FilesToDownLoad"].toString() == "All") != ui->allRadio->isChecked() ||
        obj["FilterType"].toInt() != ui->filterType->currentIndex() ||
        obj["Filter"].toString() != ui->filter->text()) {
        reloadNeeded = true;
    }
    if (ui->allRadio->isChecked()) {
        obj.insert("FilesToDownLoad",QJsonValue(QString("All")));
    } else {
        obj.insert("FilesToDownLoad",QJsonValue(QString("New")));
    }
    obj.insert("FilterType",ui->filterType->currentIndex());

    obj.insert("Filter",QJsonValue(ui->filter->text()));
    ui->filter->setEnabled(ui->filterType->currentIndex() != 0);

    /* Organize part */
    obj.insert("DownloadTo",QJsonValue(ui->DLToLine->text()));
    obj.insert("newName",QJsonValue(ui->newNameEdit->text()));
    if (obj["AllowEXIF"].toBool() == false &&
        ui->allowEXIFBox->isChecked()) {
        reloadNeeded = true;
    }
    obj.insert("AllowEXIF",QJsonValue(ui->allowEXIFBox->isChecked()));
    obj.insert("UseEXIFDate",QJsonValue(ui->UseEXIFDateBox->isChecked()));

    /* Automation */
    obj.insert("automation",QJsonValue(ui->automation->box->isChecked()));

    /* GeoTagging */
    obj.insert("GeoTag",QJsonValue(ui->GeoTagBox->isChecked()));
    if (ui->OpenPathsRadio->isChecked()) {
        obj.insert("GeoTagMode",QJsonValue(QString("OpenPaths.cc")));
    } else {
        obj.insert("GeoTagMode",QJsonValue(QString("Track Folder")));
    }
    obj.insert("TrackFolder",QJsonValue(ui->TrackFolderLine->text()));
    obj.insert("OPAccessKey",QJsonValue(ui->OPAccessKeyLine->text()));
    obj.insert("OPSecretKey",QJsonValue(ui->OPSecretKeyLine->text()));

    dc->conf[id] = obj;
    //dc->saveConfig();
    if (dpm != NULL) {
        if (reloadNeeded) {
            dpm->reloadSelection();
        } else {
            dpm->dataChanged(QModelIndex(), QModelIndex());
        }
    }
    //ui->tableView->resizeRowsToContents();
    //ui->tableView->resizeColumnsToContents();
    updateButton();
    updateStatusText();
}

void DeviceConfigView::SaveConfig() {
    qDebug() << "SaveConfig";
    if (dpm != NULL) {
        QString guessedCameraName = dpm->guessCameraName();
        if (guessedCameraName > 0) {
            QJsonObject obj = dc->conf[id].toObject();
            obj.insert("CameraName",QJsonValue(guessedCameraName));
            dc->conf[id] = obj;
        }
    }

    dc->saveConfig();
}

void DeviceConfigView::chooseDLTo() {
    QString dir = QFileDialog::getExistingDirectory(this,"Choose the directory where to download the picture files",
                                           ui->DLToLine->text());

    if (dir.size()>0) {
        ui->DLToLine->setText(dir);
        CopyToConfig();
    }
}

void DeviceConfigView::chooseTrackFolder() {
    QString dir = QFileDialog::getExistingDirectory(this,"Choose the directory containing the tracking files",
                                           ui->TrackFolderLine->text());

    if (dir.size()>0) {
        ui->TrackFolderLine->setText(dir);
        CopyToConfig();
    }
}

void DeviceConfigView::updateStatusText(){
    qint64 totalToDownload = 0;
    int nbFilesToDownload = 0;

    if (dpm != NULL) {
        dpm->getStats(&totalToDownload, &nbFilesToDownload);
    }
    ui->statusText->setText(QString("%1 files to download for a total of %2").arg(nbFilesToDownload).arg(File::size2Str(totalToDownload)));
}

void DeviceConfigView::go(){
    SaveConfig();
    if (tm != NULL) {        
        if (!dpm->getAllCom()) return;

        connect(tm,SIGNAL(downloadFinished()),this,SLOT(deleteLater()));
        ui->Tabs->setCurrentIndex(0);
        td->showProgress(tm);
        tm->launchDownloads();
    }
}


