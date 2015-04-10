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
#include "config.h"
#include "transferdialog.h"
#include <QMessageBox>
#include <QStyledItemDelegate>
#include <QHeaderView>



BoxView::BoxView(QCheckBox *check_box_, QString field_name_, bool default_value_) {
    check_box = check_box_;
    field_name = field_name_;
    default_value = default_value_;
}
void BoxView::setBox(QJsonObject *obj) {
    if (obj->value(field_name).isNull() || obj->value(field_name).isUndefined()) {
        obj->insert(field_name,QJsonValue(default_value));
    }
    check_box->setChecked(obj->value(field_name).toBool());
}
void BoxView::readBox(QJsonObject *obj) {
    obj->insert(field_name,QJsonValue(check_box->isChecked()));
}

LineEditView::LineEditView(QLineEdit *line_edit_, QString field_name_, QString default_value_){
    line_edit = line_edit_;
    field_name = field_name_;
    default_value = default_value_;
}
void LineEditView::setLine(QJsonObject *obj){
    if (obj->value(field_name).isNull() || obj->value(field_name).isUndefined()) {
        obj->insert(field_name,QJsonValue(default_value));
    }
    line_edit->setText(obj->value(field_name).toString());
}
void LineEditView::readLine(QJsonObject *obj){
    obj->insert(field_name,QJsonValue(line_edit->text()));
}

SpinBoxView::SpinBoxView(QSpinBox *spin_box_, QString field_name_, int default_value_){
    spin_box = spin_box_;
    field_name = field_name_;
    default_value = default_value_;
}
void SpinBoxView::setSpinBox(QJsonObject *obj){
    if (obj->value(field_name).isNull() || obj->value(field_name).isUndefined()) {
        obj->insert(field_name,QJsonValue(default_value));
    }
    spin_box->setValue(obj->value(field_name).toInt());
}
void SpinBoxView::readSpinBox(QJsonObject *obj){
    obj->insert(field_name,QJsonValue(spin_box->value()));
}


class CustomItemDelegate: public QStyledItemDelegate {
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const {
        int row = index.row();
        qDebug() << "sizeHint " << row << index.column();
        return QSize(30,100+10*(row%11));
    }
};

class CustomHeaderView: public QHeaderView {
public:
    CustomHeaderView(Qt::Orientation orientation, QWidget * parent = 0)
        :QHeaderView(orientation,parent){
    }

    int sectionSize(int logicalIndex) const {
        qDebug() << "sectionSize " << logicalIndex;
        return 50 + 10*(logicalIndex%5);
    }
};



DeviceConfigView::DeviceConfigView(Config *dc_, QString id_, bool editMode_, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeviceConfigView)
{
    dpm = NULL;
    tm = NULL;
    dc = dc_;
    id = id_;
    editMode = editMode_;
    pd = new QProgressDialog(this);
    td = new TransferDialog(this);
    ui->setupUi(this);
    ui->Tabs->setCurrentIndex(0);

    connect(ui->openButton, SIGNAL(clicked()),this, SLOT(chooseDLTo()));
    connect(ui->trackOpenButton, SIGNAL(clicked()),this, SLOT(chooseTrackFolder()));
    ui->tableView->setWordWrap(false);

    setAttribute(Qt::WA_DeleteOnClose, true);
    setAttribute(Qt::WA_QuitOnClose, false);
    FillWithConfig();

    if (!editMode) {
        dpm = new DownloadModel(dc, pd, editMode);
        connect(dpm, SIGNAL(itemOfInterest(QModelIndex)), this, SLOT(makeVisible(QModelIndex)));
        connect(dpm, SIGNAL(reloaded()), ui->tableView, SLOT(resizeColumnsToContents()));
        //        connect(dpm, SIGNAL(reloaded()), ui->tableView, SLOT(resizeRowsToContents()));
        connect(dpm, SIGNAL(reloaded()), this, SLOT(resizeRows()));
        ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        //ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        ui->tableView->verticalHeader()->setResizeContentsPrecision(100);
        ui->tableView->horizontalHeader()->setResizeContentsPrecision(100);
        //ui->tableView->setItemDelegate(new CustomItemDelegate());
        //ui->tableView->setVerticalHeader(new CustomHeaderView(Qt::Vertical,ui->tableView));
        connect(ui->FillCommentsButton,SIGNAL(clicked()), dpm, SLOT(getAllCom()));
        connect(dpm, SIGNAL(EXIFLoadCanceled(bool)), ui->allowEXIFBox, SLOT(setChecked(bool)));
        connect(ui->seeAvTagsButton, SIGNAL(clicked()), this, SLOT(showEXIFTags()));

        ui->tableView->setModel(dpm);
        dpm->loadPreview(id);
        tm = new TransferManager(this,dpm);
        //ui->tableView->setItemDelegate(dpm->getItemDelegate());
        //ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

        connect(ui->automation, SIGNAL(performAction()),this,SLOT(go()));
        connect(ui->free_up_simulation,SIGNAL(linkActivated(QString)),this,SLOT(handleLinks(QString)));
    } else {
        ui->goButton->setText("Save");
    }
    //setCornerWidget(new QPushButton("Go",this),Qt::BottomRightCorner);

    updateStatusText();
    updateFreeUpSimulation();

    if (!dc->LoadWindowGeometry(id,this)) {
        setWindowState(Qt::WindowMaximized);
    }
    qDebug() << "before show " << geometry();
    show();
    qDebug() << "after  show " << geometry();

    /* automation stuff */
    ui->automation->startCountDown();

}

DeviceConfigView::~DeviceConfigView() {
    qDebug() << "delete DeviceConfigView";
    qDebug() << "during destructor " << geometry();
    dc->SaveWindowGeometry(this,id);
    delete ui;
    delete dpm;
    delete pd;
}

void DeviceConfigView::resizeRows() {
    qDebug() << "resizeRows" << dpm->rowCount();
    int maxwidth = 30;
    for (int row = 0; row<dpm->rowCount(); row++) {
        QSize s = dpm->thumbnailSize(row);
        ui->tableView->setRowHeight(row,s.height());
        maxwidth = std::max(maxwidth,s.width());
    }
    ui->tableView->setColumnWidth(0,maxwidth);
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

void DeviceConfigView::FillWithConfig() {
    qDebug() << "FillWithConfig";
    QJsonObject obj = dc->devices[id].toObject();
    QString displayName = obj[CONFIG_DISPLAYNAME].toString();
    QString path = obj[CONFIG_PATH].toString();
    QString title = QCoreApplication::applicationName() + " v" + QCoreApplication::applicationVersion()
                    + " - " + displayName;
    if ( displayName != path) {
        title += " - " + path;
    }
    this->setWindowTitle(title);

    boxes << BoxView(ui->allowEXIFBox,CONFIG_ALLOWEXIF,false);
    boxes << BoxView(ui->UseEXIFDateBox,CONFIG_USEEXIFDATE,true);
    boxes << BoxView(ui->automation->box,CONFIG_AUTOMATION,false);
    boxes << BoxView(ui->GeoTagBox,CONFIG_GEOTAG,false);
    boxes << BoxView(ui->free_up_box,CONFIG_FREEUPSPACE,false);
    boxes << BoxView(ui->target_pct_box,CONFIG_TARGETPERCENTAGE,false);
    boxes << BoxView(ui->nbPics_box,CONFIG_TARGETNBPICS,true);
    boxes << BoxView(ui->protect_days_box,CONFIG_PROTECTDAYS,true);
    boxes << BoxView(ui->protect_transfer_box,CONFIG_PROTECTTRANSFER,true);

    lines << LineEditView(ui->filter,CONFIG_FILTER,obj[CONFIG_IDPATH].toString().startsWith("WPD")?"DCIM;Camera;Auto;*ANDRO;user;media;*iPhone*;*APPLE;Pictures;Screenshots;Download;WhatsApp*;Media":"");
    lines << LineEditView(ui->DLToLine,CONFIG_DOWNLOADTO,QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0));
    lines << LineEditView(ui->newNameEdit,CONFIG_NEWNAME,"yyyy-MM/yyyy-MM-dd/yyyy-MM-dd_HH-mm-ss_oName");
    lines << LineEditView(ui->TrackFolderLine,CONFIG_TRACKFOLDER,"");
    lines << LineEditView(ui->OPAccessKeyLine,CONFIG_OPACCESSKEY,"");
    lines << LineEditView(ui->OPSecretKeyLine,CONFIG_OPSECRETKEY,"");
    lines << LineEditView(ui->nbPics,CONFIG_NBPICS,"200");

    spinBoxes << SpinBoxView(ui->target_pct,CONFIG_TARGETPERCENTAGEVALUE,25);
    spinBoxes << SpinBoxView(ui->protect_days,CONFIG_PROTECTDAYSVALUE,30);


    /* Download part */
    if (obj[CONFIG_FILESTODOWNLOAD].isNull() || obj[CONFIG_FILESTODOWNLOAD].isUndefined()) {
        obj.insert(CONFIG_FILESTODOWNLOAD,QJsonValue(QString("New")));
    }
    if (obj[CONFIG_FILESTODOWNLOAD].toString() == "All") {
        ui->allRadio->setChecked(true);
    } else {
        ui->newRadio->setChecked(true);
    }

    if (obj[CONFIG_FILTERTYPE].isNull() || obj[CONFIG_FILTERTYPE].isUndefined()) {
        if (obj[CONFIG_IDPATH].toString().startsWith("WPD")) {
            obj.insert(CONFIG_FILTERTYPE,QJsonValue(2));
        } else {
            obj.insert(CONFIG_FILTERTYPE,QJsonValue(0));
        }
    }
    ui->filterType->setCurrentIndex(obj[CONFIG_FILTERTYPE].toInt());

    /* Organize part */

    /* Geo Tagging */
    if (obj[CONFIG_GEOTAGMODE].isNull() || obj[CONFIG_GEOTAGMODE].isUndefined()) {
        obj.insert(CONFIG_GEOTAGMODE,QJsonValue(QString("Track Folder")));
    }
    if (obj[CONFIG_GEOTAGMODE].toString() == "OpenPaths.cc") {
        ui->OpenPathsRadio->setChecked(true);
    } else {
        ui->TrackFolderRadio->setChecked(true);
    }

    for (int i = 0; i < boxes.size(); i++) {
        boxes[i].setBox(&obj);
        connect(boxes.at(i).check_box,SIGNAL(clicked()),this,SLOT(CopyToConfig()));
    }
    for (int i = 0; i < lines.size(); i++) {
        lines[i].setLine(&obj);
        connect(lines.at(i).line_edit,SIGNAL(textEdited(QString)),this,SLOT(CopyToConfig()));
    }
    for (int i = 0; i < spinBoxes.size(); i++) {
        spinBoxes[i].setSpinBox(&obj);
        connect(spinBoxes.at(i).spin_box,SIGNAL(valueChanged(int)),this,SLOT(CopyToConfig()));
    }

    dc->devices[id] = obj;
    dc->saveDevices();

    //ui->tableView->resizeRowsToContents();

    updateButton();

    if (obj[CONFIG_IDPATH].toString().startsWith("WPD")) {
        ui->free_up_box->setEnabled(false);
    }
    ui->OpenPathsRadio->setEnabled(false);
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
    ui->filter->setEnabled(ui->filterType->currentIndex() != 0);
}

void DeviceConfigView::CopyToConfig() {
    qDebug() << "CopyToConfig";
    QJsonObject obj = dc->devices[id].toObject();
    bool reloadNeeded = false;
    bool geotaggerUpdateNeeded = false;

    /* Detect config modification that trigger actions */
    if ((obj[CONFIG_FILESTODOWNLOAD].toString() == "All") != ui->allRadio->isChecked() ||
        obj[CONFIG_FILTERTYPE].toInt() != ui->filterType->currentIndex() ||
        obj[CONFIG_FILTER].toString() != ui->filter->text()) {
        reloadNeeded = true;
    }
    if (obj[CONFIG_ALLOWEXIF].toBool() == false &&
        ui->allowEXIFBox->isChecked()) {
        reloadNeeded = true;
    }
    if (obj[CONFIG_GEOTAG].toBool() != ui->GeoTagBox->isChecked() ||
        obj[CONFIG_TRACKFOLDER].toString() != ui->TrackFolderLine->text()) {
        geotaggerUpdateNeeded = true;
    }

    /* update non-standard fields */
    if (ui->allRadio->isChecked()) {
        obj.insert(CONFIG_FILESTODOWNLOAD,QJsonValue(QString("All")));
    } else {
        obj.insert(CONFIG_FILESTODOWNLOAD,QJsonValue(QString("New")));
    }
    obj.insert(CONFIG_FILTERTYPE,ui->filterType->currentIndex());

    if (ui->OpenPathsRadio->isChecked()) {
        obj.insert(CONFIG_GEOTAGMODE,QJsonValue(QString("OpenPaths.cc")));
    } else {
        obj.insert(CONFIG_GEOTAGMODE,QJsonValue(QString("Track Folder")));
    }

    /* update standard fields */
    for (int i = 0; i < boxes.size(); i++) {
        boxes[i].readBox(&obj);
    }
    for (int i = 0; i < lines.size(); i++) {
        lines[i].readLine(&obj);
    }
    for (int i = 0; i < spinBoxes.size(); i++) {
        spinBoxes[i].readSpinBox(&obj);
    }

    dc->devices[id] = obj;
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
    if (ui->Tabs->currentIndex() == 2 || reloadNeeded) {
        updateFreeUpSimulation();
    }

    updateButton();
    updateStatusText();

    if (tm != NULL) {
        if (geotaggerUpdateNeeded) {
            tm->udpateGeoTagger();
        }
    }
}

void DeviceConfigView::SaveConfig() {
    qDebug() << "SaveConfig";
    if (dpm != NULL) {
        QString guessedCameraName = dpm->guessCameraName();
        if (guessedCameraName > 0) {
            QJsonObject obj = dc->devices[id].toObject();
            obj.insert(CONFIG_CAMERANAME,QJsonValue(guessedCameraName));
            dc->devices[id] = obj;
        }
    }

    dc->saveDevices();
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
    ui->statusText->setText(QString("%1 files are selected for download (%2)").arg(nbFilesToDownload).arg(File::size2Str(totalToDownload)));
}

void DeviceConfigView::updateFreeUpSimulation() {
    qint64 targetAvailable = 0;
    qint64 bytesDeleted = 0;
    int nbFilesDeleted = 0;
    qint64 available = dc->devices[id].toObject()[CONFIG_BYTESAVAILABLE].toString().toLongLong();
    qint64 totalSpace = dc->devices[id].toObject()[CONFIG_DEVICESIZE].toString().toLongLong();
    if (dpm != NULL) {
        int nbPics = ui->nbPics->text().toInt();
        dpm->freeUpSpace(true,&targetAvailable,&bytesDeleted,&nbFilesDeleted);
        ui->free_up_nbpics_label->setText(QString("pictures. 1 picture = %1, so %2 pictures = %3")
                                          .arg(File::size2Str(dpm->averagePicSize))
                                          .arg(nbPics)
                                          .arg(File::size2Str(dpm->averagePicSize*nbPics)));
    }

    ui->free_up_simulation->setText(QString("<html><head/><body><p>%1 (%2\%)<br/>%3 (<a href=\"files to delete\"><span style=\" text-decoration: underline; color:#0000ff;\">%4 files</span></a>)<br/>%5 (%6\%)<br/>%7 (%8\%)</p></body></html>")
                                    .arg(File::size2Str(available))
                                    .arg((float)available*100/(float)totalSpace,0,'f',1)
                                    .arg(File::size2Str(bytesDeleted))
                                    .arg(nbFilesDeleted)
                                    .arg(File::size2Str(available+bytesDeleted))
                                    .arg((float)(available+bytesDeleted)*100/(float)totalSpace,0,'f',1)
                                    .arg(File::size2Str(targetAvailable))
                                    .arg((float)targetAvailable*100/(float)totalSpace,0,'f',1));

}


void DeviceConfigView::go(){
    SaveConfig();
    if (tm != NULL) {        
        if (!dpm->getAllCom()) return;

        connect(tm,SIGNAL(downloadFinished()),this,SLOT(postDownloadActions()));
        ui->Tabs->setCurrentIndex(0);
        td->showProgress(tm);
        tm->launchDownloads();

        QJsonObject obj = dc->devices[id].toObject();
        obj.insert(CONFIG_LASTTRANSFER,QJsonValue(QString("%1").arg(QDateTime::currentDateTime().toTime_t())));
        dc->devices[id] = obj;
        dc->deviceFieldChanged(id);
        SaveConfig();
    } else {
        deleteLater();
    }
}

void DeviceConfigView::postDownloadActions(){
    if (dpm != NULL) {
        qint64 targetAvailable = 0;
        qint64 bytesDeleted = 0;
        int nbFilesDeleted = 0;
        dpm->freeUpSpace(true,&targetAvailable,&bytesDeleted,&nbFilesDeleted);

        if (nbFilesDeleted > 0) {
            td->hide();
            ui->Tabs->setCurrentIndex(2);
            QMessageBox mb(QMessageBox::Question,
                           QCoreApplication::applicationName(),
                           QString("PicsDL is about to delete %1 files (%2) from your device.\nAre you sure?")
                           .arg(nbFilesDeleted)
                           .arg(File::size2Str(bytesDeleted)),
                           QMessageBox::Yes | QMessageBox::No);
            int answer = mb.exec();
            if (answer == QMessageBox::Yes) {
                dpm->freeUpSpace(false,&targetAvailable,&bytesDeleted,&nbFilesDeleted);
            }
        }
    }
    deleteLater();
}


void DeviceConfigView::handleLinks(QString link){
    qDebug() << QString("handleLinks(%1)").arg(link);
    if (dpm != NULL) {
        QString file_list_str;
        for (int i = 0; i < dpm->deletedFiles.size(); i++) {
            file_list_str += dpm->deletedFiles.at(i)->absoluteFilePath + "\n";
        }
        if (dpm->deletedFiles.size() == 0) {
            file_list_str += "None!";
        }

        QMessageBox mb(QMessageBox::Information,
                       "Files to delete",
                       file_list_str,
                       QMessageBox::Ok);
        mb.exec();
    }
}
