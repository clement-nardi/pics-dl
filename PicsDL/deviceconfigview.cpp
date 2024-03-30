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
#include <QHeaderView>
#include <QMenu>
#include <QGroupBox>
#include "newnameselection.h"


BoxView::BoxView(QCheckBox *check_box_, QString field_name_, bool default_value_, DeviceConfigView *dcv_)
    :QObject(dcv_){
    check_box = check_box_;
    field_name = field_name_;
    default_value = default_value_;
    dcv = dcv_;
    setBox();
    connect(check_box,SIGNAL(toggled(bool)),this,SLOT(readBox()));
}
void BoxView::setBox() {
    QJsonObject obj = dcv->dc->devices[dcv->id].toObject();
    if (obj.value(field_name).isNull() || obj.value(field_name).isUndefined()) {
        obj.insert(field_name,QJsonValue(default_value));
        dcv->dc->devices[dcv->id] = obj;
    }
    check_box->setChecked(obj.value(field_name).toBool());
}
void BoxView::readBox() {
    QJsonObject obj = dcv->dc->devices[dcv->id].toObject();
    obj.insert(field_name,QJsonValue(check_box->isChecked()));
    dcv->dc->devices[dcv->id] = obj;
}

GroupBoxView::GroupBoxView(QGroupBox *group_box_, QString field_name_, bool default_value_, DeviceConfigView *dcv_)
    :QObject(dcv_){
    group_box = group_box_;
    field_name = field_name_;
    default_value = default_value_;
    dcv = dcv_;
    setBox();
    connect(group_box,SIGNAL(toggled(bool)),this,SLOT(readBox()));
}
void GroupBoxView::setBox() {
    QJsonObject obj = dcv->dc->devices[dcv->id].toObject();
    if (obj.value(field_name).isNull() || obj.value(field_name).isUndefined()) {
        obj.insert(field_name,QJsonValue(default_value));
        dcv->dc->devices[dcv->id] = obj;
    }
    group_box->setChecked(obj.value(field_name).toBool());
}
void GroupBoxView::readBox() {
    QJsonObject obj = dcv->dc->devices[dcv->id].toObject();
    obj.insert(field_name,QJsonValue(group_box->isChecked()));
    dcv->dc->devices[dcv->id] = obj;
}

LineEditView::LineEditView(QLineEdit *line_edit_, QString field_name_, QString default_value_, DeviceConfigView *dcv_)
    :QObject(dcv_){
    line_edit = line_edit_;
    field_name = field_name_;
    default_value = default_value_;
    dcv = dcv_;
    setLine();
    connect(line_edit,SIGNAL(textChanged(QString)),this,SLOT(readLine()));
}
void LineEditView::setLine(){
    QJsonObject obj = dcv->dc->devices[dcv->id].toObject();
    if (obj.value(field_name).isNull() || obj.value(field_name).isUndefined()) {
        obj.insert(field_name,QJsonValue(default_value));
        dcv->dc->devices[dcv->id] = obj;
    }
    line_edit->setText(obj.value(field_name).toString());
}
void LineEditView::readLine(){
    QJsonObject obj = dcv->dc->devices[dcv->id].toObject();
    obj.insert(field_name,QJsonValue(line_edit->text()));
    dcv->dc->devices[dcv->id] = obj;
}

SpinBoxView::SpinBoxView(QSpinBox *spin_box_, QString field_name_, int default_value_, DeviceConfigView *dcv_)
    :QObject(dcv_){
    spin_box = spin_box_;
    field_name = field_name_;
    default_value = default_value_;
    dcv = dcv_;
    setSpinBox();
    connect(spin_box,SIGNAL(valueChanged(int)),this,SLOT(readSpinBox()));
}
void SpinBoxView::setSpinBox(){
    QJsonObject obj = dcv->dc->devices[dcv->id].toObject();
    if (obj.value(field_name).isNull() || obj.value(field_name).isUndefined()) {
        obj.insert(field_name,QJsonValue(default_value));
        dcv->dc->devices[dcv->id] = obj;
    }
    spin_box->setValue(obj.value(field_name).toInt());
}
void SpinBoxView::readSpinBox(){
    QJsonObject obj = dcv->dc->devices[dcv->id].toObject();
    obj.insert(field_name,QJsonValue(spin_box->value()));
    dcv->dc->devices[dcv->id] = obj;
}



DeviceConfigView::DeviceConfigView(Config *dc_, QString id_, bool editMode_, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeviceConfigView),
    do_not_download_action(QIcon(":/icons/remove"),tr("Do not download"),this),
    download_action(QIcon(":/icons/download"),tr("Download"),this)
{
    dpm = NULL;
    tm = NULL;
    dc = dc_;
    id = id_;
    editMode = editMode_;

    /* init the widget after the constructor has returned */
    connect(this, SIGNAL(initLater()),this,SLOT(init()),Qt::QueuedConnection);
    emit initLater();
}

void DeviceConfigView::init() {
    pd = new QProgressDialog(this);
    td = new TransferDialog(this);
    ui->setupUi(this);
    QJsonObject obj = dc->devices[id].toObject();
    NewNameSelection *nns = new NewNameSelection(this);
    connect(ui->choose_new_name,SIGNAL(clicked()),nns,SLOT(show()));
    connect(nns,SIGNAL(newNameSelected(QString)),this,SLOT(setNewNamePattern(QString)));

    ui->keywordsTable->resizeColumnsToContents();
    ui->keywordsTable->setMinimumWidth(ui->keywordsTable->horizontalHeader()->sectionSize(0) +
                                       ui->keywordsTable->horizontalHeader()->sectionSize(1) + 2);

    /* setup tabs (Qt Designer does not support QTabBar) */
    ui->tabs->insertTab(TAB_SELECT      ,QIcon(":/icons/download"),tr("&Select && Download"));
    ui->tabs->insertTab(TAB_ORGANIZE    ,QIcon(":/icons/tree"),tr("&Organize"));
    ui->tabs->insertTab(TAB_GEOTAG      ,QIcon(":/icons/gps"),tr("&Geotag"));
    ui->tabs->insertTab(TAB_FREEUPSPACE ,QIcon(":/icons/delete"),tr("&Free Up Space"));
    connect(ui->tabs,SIGNAL(currentChanged(int)),this,SLOT(handleTabChange(int)));

    tableMenu.addAction(&do_not_download_action);
    tableMenu.addAction(&download_action);

    connect(ui->openButton, SIGNAL(clicked()),this, SLOT(chooseDLTo()));
    connect(ui->trackOpenButton, SIGNAL(clicked()),this, SLOT(chooseTrackFolder()));
    ui->tableView->setWordWrap(false);

    connect(ui->GeoTagBox,SIGNAL(toggled(bool)),this, SLOT(geotagToggled(bool)));

    setAttribute(Qt::WA_DeleteOnClose, true);
    setAttribute(Qt::WA_QuitOnClose, false);
    FillWithConfig();

    if (!editMode) {
        dpm = new DownloadModel(dc, pd, editMode);
        connect(dpm, SIGNAL(itemOfInterest(QModelIndex)), this, SLOT(makeVisible(QModelIndex)));
        //        connect(dpm, SIGNAL(reloaded()), ui->tableView, SLOT(resizeRowsToContents()));
        connect(dpm, SIGNAL(reloaded()), this, SLOT(resizeRows()));
        connect(dpm, SIGNAL(reloaded()), ui->tableView, SLOT(resizeColumnsToContents()));
        connect(dpm, SIGNAL(reloaded()), this, SLOT(updateStatusText()));
        connect(dpm, SIGNAL(selectionModified()), this, SLOT(updateStatusText()));
        ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        ui->tableView->horizontalHeader()->setResizeContentsPrecision(50);
        connect(ui->FillCommentsButton,SIGNAL(clicked()), dpm, SLOT(getAllCom()));
        connect(dpm, SIGNAL(EXIFLoadCanceled(bool)), ui->allowEXIFBox, SLOT(setChecked(bool)));
        connect(ui->seeAvTagsButton, SIGNAL(clicked()), this, SLOT(showEXIFTags()));

        ui->tableView->horizontalHeader()->setSortIndicator(COL_DATE,Qt::AscendingOrder);
        ui->tableView->setModel(dpm);
        dpm->loadPreview(id);
        tm = new TransferManager(this,dpm);

        connect(ui->automation, SIGNAL(performAction()),this,SLOT(go()));
        connect(ui->free_up_simulation,SIGNAL(linkActivated(QString)),this,SLOT(handleLinks(QString)));

        reloadTimer.setSingleShot(true);
        reloadTimer.setInterval(1000);
        connect(&reloadTimer,SIGNAL(timeout()),dpm,SLOT(reloadSelection()));

        ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(ui->tableView,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(showTableContextMenu(QPoint)));
        connect(&download_action,SIGNAL(triggered()),this,SLOT(download_handle()));
        connect(&do_not_download_action,SIGNAL(triggered()),this,SLOT(do_not_download_handle()));

        /* Setup impacts on the model */
        /* instant reload */
        connect(ui->allRadio,SIGNAL(toggled(bool)),dpm,SLOT(reloadSelection()));
        connect(ui->filterType,SIGNAL(currentIndexChanged(int)),dpm,SLOT(reloadSelection()));
        connect(ui->limit_depth_box,SIGNAL(toggled(bool)),dpm,SLOT(reloadSelection()));
        connect(ui->allowEXIFBox,SIGNAL(toggled(bool)),dpm,SLOT(reloadSelection()));
        connect(ui->pictures_box,SIGNAL(toggled(bool)),dpm,SLOT(reloadSelection()));
        connect(ui->videos_box,SIGNAL(toggled(bool)),dpm,SLOT(reloadSelection()));
        connect(ui->others_box,SIGNAL(toggled(bool)),dpm,SLOT(reloadSelection()));
        connect(ui->limit_depth_spin,SIGNAL(valueChanged(int)),dpm,SLOT(reloadSelection()));

        /* delayed reload */
        connect(ui->filter,SIGNAL(textChanged(QString)),&reloadTimer,SLOT(start()));
        connect(ui->others_patterns,SIGNAL(textChanged(QString)),&reloadTimer,SLOT(start()));

        /* destination */
        connect(ui->allowEXIFBox,SIGNAL(toggled(bool)),dpm,SLOT(updateNewPaths()));
        connect(ui->UseEXIFDateBox,SIGNAL(toggled(bool)),dpm,SLOT(updateNewPaths()));
        connect(ui->newNameEdit,SIGNAL(textChanged(QString)),dpm,SLOT(updateNewPaths()));
        connect(ui->DLToLine,SIGNAL(textChanged(QString)),dpm,SLOT(updateNewPaths()));
        connect(ui->overwrite_destination_box,SIGNAL(toggled(bool)),dpm,SLOT(updateNewPaths())); /* impact in tool-tip */
        connect(ui->move_instead_of_copy_box,SIGNAL(toggled(bool)),dpm,SLOT(updateNewPaths())); /* impact in tool-tip */
        connect(ui->GeoTagBox,SIGNAL(toggled(bool)),dpm,SLOT(updateNewPaths())); /* impact on decoration */

        /* impact on the geotagger */
        connect(ui->GeoTagBox,SIGNAL(toggled(bool)),tm,SLOT(updateGeoTagger()));
        connect(ui->TrackFolderLine,SIGNAL(textChanged(QString)),tm,SLOT(updateGeoTagger()));
        connect(ui->TrackFolderRadio,SIGNAL(toggled(bool)),tm,SLOT(updateGeoTagger()));


    } else {
        ui->goButton->setText(tr("Save"));
    }

    updateStatusText();

    if (!dc->LoadWindowGeometry(id,this)) {
        setWindowState(Qt::WindowMaximized);
    }

    if (obj[CONFIG_NEWNAME].toString() == "oFolder1-/oName" &&
        obj[CONFIG_PATH].toString() == obj[CONFIG_DOWNLOADTO].toString()) {
        ui->tabs->setCurrentIndex(TAB_GEOTAG);
    } else {
        ui->tabs->setCurrentIndex(TAB_ORGANIZE);
    }
    show();

    /* automation stuff */
    ui->automation->startCountDown();

}

DeviceConfigView::~DeviceConfigView() {
    qDebug() << "delete DeviceConfigView";
    qDebug() << "during destructor " << geometry();
    dc->saveWindowGeometry(this,id);
    saveConfig();
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

    /* special widget connections */
    connect(ui->filterType,SIGNAL(currentIndexChanged(int)),this,SLOT(updateButton()));
    connect(ui->newNameEdit,SIGNAL(textChanged(QString)),this,SLOT(updateButton()));

    /* Initialize special widgets (comboBox and radioButton) */
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

    if (obj[CONFIG_GEOTAGMODE].isNull() || obj[CONFIG_GEOTAGMODE].isUndefined()) {
        obj.insert(CONFIG_GEOTAGMODE,QJsonValue(QString("Track Folder")));
    }
    if (obj[CONFIG_GEOTAGMODE].toString() == "OpenPaths.cc") {
        ui->OpenPathsRadio->setChecked(true);
    } else {
        ui->TrackFolderRadio->setChecked(true);
    }

    dc->devices[id] = obj;

    /* special widgets synchronisation with the JSON config */
    connect(ui->allRadio,SIGNAL(toggled(bool)),this,SLOT(readSpecialWidgets()));
    connect(ui->filterType,SIGNAL(currentIndexChanged(int)),this,SLOT(readSpecialWidgets()));
    connect(ui->TrackFolderRadio,SIGNAL(toggled(bool)),this,SLOT(readSpecialWidgets()));

    /* initialze and setup standard widgets */
    checkBoxes << new BoxView(ui->allowEXIFBox,CONFIG_ALLOWEXIF,false,this);
    checkBoxes << new BoxView(ui->UseEXIFDateBox,CONFIG_USEEXIFDATE,true,this);
    checkBoxes << new BoxView(ui->automation->box,CONFIG_AUTOMATION,false,this);
    checkBoxes << new BoxView(ui->GeoTagBox,CONFIG_GEOTAG,false,this);
    checkBoxes << new BoxView(ui->free_up_box,CONFIG_FREEUPSPACE,false,this);
    checkBoxes << new BoxView(ui->target_pct_box,CONFIG_TARGETPERCENTAGE,false,this);
    checkBoxes << new BoxView(ui->nbPics_box,CONFIG_TARGETNBPICS,true,this);
    checkBoxes << new BoxView(ui->protect_days_box,CONFIG_PROTECTDAYS,true,this);
    checkBoxes << new BoxView(ui->protect_transfer_box,CONFIG_PROTECTTRANSFER,true,this);
    checkBoxes << new BoxView(ui->limit_depth_box,CONFIG_LIMITDEPTH,false,this);
    checkBoxes << new BoxView(ui->pictures_box,CONFIG_PICTUREFILES,true,this);
    checkBoxes << new BoxView(ui->videos_box,CONFIG_VIDEOFILES,true,this);
    checkBoxes << new BoxView(ui->move_instead_of_copy_box,CONFIG_MOVEFILES,false,this);
    checkBoxes << new BoxView(ui->overwrite_destination_box,CONFIG_OVERWRITEFILES,false,this);

    groupBoxes << new GroupBoxView(ui->others_box,CONFIG_OTHERFILES,false,this);

    lines << new LineEditView(ui->filter,CONFIG_FILTER,obj[CONFIG_IDPATH].toString().startsWith("WPD")?"DCIM;Camera;Auto;*ANDRO;user;media;*iPhone*;*APPLE;Pictures;Screenshots;Download;WhatsApp*;Media":"",this);
    lines << new LineEditView(ui->DLToLine,CONFIG_DOWNLOADTO,QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0),this);
    lines << new LineEditView(ui->newNameEdit,CONFIG_NEWNAME,"yyyy-MM/yyyy-MM-dd/yyyy-MM-dd_HH-mm-ss_oName",this);
    lines << new LineEditView(ui->TrackFolderLine,CONFIG_TRACKFOLDER,"",this);
    lines << new LineEditView(ui->OPAccessKeyLine,CONFIG_OPACCESSKEY,"",this);
    lines << new LineEditView(ui->OPSecretKeyLine,CONFIG_OPSECRETKEY,"",this);
    lines << new LineEditView(ui->nbPics,CONFIG_NBPICS,"200",this);
    lines << new LineEditView(ui->others_patterns,CONFIG_OTHERFILESPATTERNS,"*.gpx;*.kml;*.xml",this);

    spinBoxes << new SpinBoxView(ui->target_pct,CONFIG_TARGETPERCENTAGEVALUE,25,this);
    spinBoxes << new SpinBoxView(ui->protect_days,CONFIG_PROTECTDAYSVALUE,30,this);
    spinBoxes << new SpinBoxView(ui->limit_depth_spin,CONFIG_DEPTHLIMIT,0,this);

    connect(ui->free_up_box,SIGNAL(toggled(bool)),this,SLOT(updateFreeUpSimulation()));
    connect(ui->target_pct_box,SIGNAL(toggled(bool)),this,SLOT(updateFreeUpSimulation()));
    connect(ui->nbPics_box,SIGNAL(toggled(bool)),this,SLOT(updateFreeUpSimulation()));
    connect(ui->protect_days_box,SIGNAL(toggled(bool)),this,SLOT(updateFreeUpSimulation()));
    connect(ui->protect_transfer_box,SIGNAL(toggled(bool)),this,SLOT(updateFreeUpSimulation()));
    connect(ui->target_pct,SIGNAL(valueChanged(int)),this,SLOT(updateFreeUpSimulation()));
    connect(ui->nbPics,SIGNAL(textChanged(QString)),this,SLOT(updateFreeUpSimulation()));
    connect(ui->protect_days,SIGNAL(valueChanged(int)),this,SLOT(updateFreeUpSimulation()));


    dc->saveDevices();

    if (obj[CONFIG_IDPATH].toString().startsWith("WPD")) {
        QString tooltip = tr("This feature is not available for devices connected via tha MTP or PTP protocols");
        ui->free_up_box->setEnabled(false);
        ui->free_up_box->setToolTip(tooltip);
        ui->move_instead_of_copy_box->setEnabled(false);
        ui->move_instead_of_copy_box->setToolTip(tooltip);
    }
    if (obj[CONFIG_DEVICETYPE].toString() == "Folder") {
        ui->free_up_box->setEnabled(false);
        ui->free_up_box->setToolTip(tr("This feature is not available for folders"));
    }

    ui->OpenPathsRadio->setEnabled(false);
    ui->OpenPathsRadio->setToolTip(tr("This feature is not implemented yet"));
}


void DeviceConfigView::updateButton(){
    qDebug() << "updateButton";
    if (ui->newNameEdit->text().contains("dCom") ||
        ui->newNameEdit->text().contains("sCom") ) {
        ui->FillCommentsButton->setEnabled(true);
    } else {
        ui->FillCommentsButton->setDisabled(true);
    }
    ui->filter->setEnabled(ui->filterType->currentIndex() != 0);
}

void DeviceConfigView::readSpecialWidgets() {
    qDebug() << "CopyToConfig";
    QJsonObject obj = dc->devices[id].toObject();

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

    dc->devices[id] = obj;
}

void DeviceConfigView::saveConfig() {
    qDebug() << "SaveConfig";
    if (dpm != NULL) {
        QString guessedCameraName = dpm->guessCameraName();
        if (guessedCameraName.size() > 0) {
            QJsonObject obj = dc->devices[id].toObject();
            obj.insert(CONFIG_CAMERANAME,QJsonValue(guessedCameraName));
            dc->devices[id] = obj;
        }
    }

    dc->saveDevices();
}

void DeviceConfigView::chooseDLTo() {
    QString dir = QFileDialog::getExistingDirectory(this,tr("Choose the directory where to download the picture files"),
                                           ui->DLToLine->text());

    if (dir.size()>0) {
        ui->DLToLine->setText(dir);
        readSpecialWidgets();
    }
}

void DeviceConfigView::chooseTrackFolder() {
    QString dir = QFileDialog::getExistingDirectory(this,tr("Choose the directory containing the tracking files"),
                                           ui->TrackFolderLine->text());

    if (dir.size()>0) {
        ui->TrackFolderLine->setText(dir);
        readSpecialWidgets();
    }
}

void DeviceConfigView::updateStatusText(){
    qint64 totalToDownload = 0;
    int nbFilesToDownload = 0;

    if (dpm != NULL) {
        dpm->getStats(&totalToDownload, &nbFilesToDownload);
    }
    ui->statusText->setText(QString(tr("%1 files are selected for download (%2)")).arg(nbFilesToDownload).arg(File::size2Str(totalToDownload)));
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
        ui->free_up_nbpics_label->setText(QString(tr("pictures. 1 picture = %1, so %2 pictures = %3"))
                                          .arg(File::size2Str(dpm->averagePicSize))
                                          .arg(nbPics)
                                          .arg(File::size2Str(dpm->averagePicSize*nbPics)));
    }

    ui->free_up_simulation->setText(QString("<html><head/><body><p>%1 (%2\%)<br/>%3 (<a href=\"" + tr("files to delete") + "\"><span style=\" text-decoration: underline; color:#0000ff;\">%4 " + tr("files") + "</span></a>)<br/>%5 (%6\%)<br/>%7 (%8\%)</p></body></html>")
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
    saveConfig();
    if (tm != NULL) {        
        if (!dpm->getAllCom()) return;

        connect(tm,SIGNAL(downloadFinished()),this,SLOT(postDownloadActions()));
        ui->tabs->setCurrentIndex(TAB_ORGANIZE);
        td->showProgress(tm);
        tm->launchDownloads();

        QJsonObject obj = dc->devices[id].toObject();
        obj.insert(CONFIG_LASTTRANSFER,QJsonValue(QString("%1").arg(QDateTime::currentDateTime().toSecsSinceEpoch())));
        dc->devices[id] = obj;
        dc->deviceFieldChanged(id);
        saveConfig();
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
            ui->tabs->setCurrentIndex(TAB_FREEUPSPACE);
            QMessageBox mb(QMessageBox::Question,
                           QCoreApplication::applicationName(),
                           QString(tr("PicsDL is about to delete %1 files (%2) from your device.\nAre you sure?"))
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
            file_list_str += dpm->deletedFiles.at(i)->absoluteFilePath() + "\n";
        }
        if (dpm->deletedFiles.size() == 0) {
            file_list_str += tr("None!");
        }

        QMessageBox mb(QMessageBox::Information,
                       tr("Files to delete"),
                       file_list_str,
                       QMessageBox::Ok);
        mb.exec();
    }
}


void DeviceConfigView::showTableContextMenu(QPoint p){
    qDebug() << "showTableContextMenu" << p;
    QModelIndexList rows = ui->tableView->selectionModel()->selectedRows();
    if (rows.size() > 0) {
        tableMenu.popup(ui->tableView->mapToGlobal(p));
    }
}

void DeviceConfigView::download_handle(){
    QModelIndexList rows = ui->tableView->selectionModel()->selectedRows();
    dpm->markForDownload(rows,true);
}

void DeviceConfigView::do_not_download_handle(){
    QModelIndexList rows = ui->tableView->selectionModel()->selectedRows();
    dpm->markForDownload(rows,false);
}

#define L1_WITH_TABLEVIEW       1
#define L1_WITHOUT_TABLEVIEW    0
#define L2_SELECT               0
#define L2_ORGANIZE             1
#define L2_GEOTAG               2

void DeviceConfigView::handleTabChange(int idx){
    qDebug() << "handleTabChange" << idx;
    switch (idx) {
    case TAB_SELECT:
        ui->level_1_stack->setCurrentIndex(L1_WITH_TABLEVIEW);
        ui->level_2_stack->setCurrentIndex(L2_SELECT);
        ui->tableView->setColumnHidden(COL_FILEPATH,false);
        ui->tableView->setColumnHidden(COL_FILENAME,true);
        ui->tableView->setColumnHidden(COL_SIZE,false);
        ui->tableView->setColumnHidden(COL_MODIFIED,false);
        ui->tableView->setColumnHidden(COL_DATE,true);
        ui->tableView->setColumnHidden(COL_GPS,true);
        ui->tableView->setColumnHidden(COL_NEWPATH,false);
        break;
    case TAB_ORGANIZE:
        ui->level_1_stack->setCurrentIndex(L1_WITH_TABLEVIEW);
        ui->level_2_stack->setCurrentIndex(L2_ORGANIZE);
        ui->tableView->setColumnHidden(COL_FILEPATH,true);
        ui->tableView->setColumnHidden(COL_FILENAME,false);
        ui->tableView->setColumnHidden(COL_SIZE,true);
        ui->tableView->setColumnHidden(COL_MODIFIED,false);
        ui->tableView->setColumnHidden(COL_DATE,true);
        ui->tableView->setColumnHidden(COL_GPS,true);
        ui->tableView->setColumnHidden(COL_NEWPATH,false);
        break;
    case TAB_GEOTAG:
        ui->level_1_stack->setCurrentIndex(L1_WITH_TABLEVIEW);
        ui->level_2_stack->setCurrentIndex(L2_GEOTAG);
        ui->tableView->setColumnHidden(COL_FILEPATH,true);
        ui->tableView->setColumnHidden(COL_FILENAME,false);
        ui->tableView->setColumnHidden(COL_SIZE,true);
        ui->tableView->setColumnHidden(COL_MODIFIED,true);
        ui->tableView->setColumnHidden(COL_DATE,false);
        ui->tableView->setColumnHidden(COL_GPS,false);
        ui->tableView->setColumnHidden(COL_NEWPATH,true);
        break;
    case TAB_FREEUPSPACE:
        ui->level_1_stack->setCurrentIndex(L1_WITHOUT_TABLEVIEW);
        updateFreeUpSimulation();
        break;
    default:
        qDebug() << "Unknown tab index!!";
        break;
    }
}

void DeviceConfigView::setNewNamePattern(QString pattern) {
    ui->newNameEdit->setText(pattern);
}

void DeviceConfigView::geotagToggled(bool checked){
    if (checked) {
        ui->allowEXIFBox->setChecked(true);
    }
    ui->allowEXIFBox->setEnabled(!checked);
}
