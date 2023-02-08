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

#include "downloadmodel.h"
#include <QDateTime>
#include <QDebug>
#include <QIcon>
#include <QVector>
#include <QThread>
#include <QApplication>
#include <QInputDialog>
#include <QTableWidget>
#include "dcomdialog.h"
#include "exifdialog.h"
#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QCryptographicHash>
#include "geotagger.h"
#include <QGeoCoordinate>


static int thumbnailSurface(File *fi) {
    QPixmap *tn = fi->getThumbnail();
    if (!tn->isNull()) {
        return tn->size().width()*tn->size().height();
    } else {
        return 30*30;
    }
}

static DownloadModel *dpm_s;

#define IF_DIFFERENT_RETURN(a,b,asc) {if ((a) != (b)) {return (asc)?((a)<(b)):((b)<(a));}}

static bool fileLessThan(File *a, File *b) {
    for (int i = 0; i < dpm_s->sortColumnOrder.size(); i++) {
        int column = dpm_s->sortColumnOrder.at(i);
        bool isAscending = dpm_s->sortIsAscending[column];
        switch (column) {
        case COL_THUMBNAIL: IF_DIFFERENT_RETURN(thumbnailSurface(a) ,thumbnailSurface(b),isAscending) break;
        case COL_FILEPATH:  IF_DIFFERENT_RETURN(a->absoluteFilePath(),b->absoluteFilePath(),isAscending) break;
        case COL_FILENAME:  IF_DIFFERENT_RETURN(a->fileName()       ,b->fileName()      ,isAscending) break;
        case COL_SIZE:      IF_DIFFERENT_RETURN(a->size             ,b->size            ,isAscending) break;
        case COL_DATE:      IF_DIFFERENT_RETURN(a->lastModified     ,b->lastModified    ,isAscending) break;
        case COL_NEWPATH:   IF_DIFFERENT_RETURN(dpm_s->newPath(a)   ,dpm_s->newPath(b)  ,isAscending) break;
        }
    }
    return 0;
}


static QStringList dateKeywords = QString("yyyy;yy;MMMM;MMM;MM;dddd;ddd;dd;HH;mm;ss").split(";");

DownloadModel::DownloadModel(Config *dc_, QProgressDialog *pd_, bool editMode_, QObject *parent) :
    QAbstractTableModel(parent)
{
    dc = dc_;
    editMode = editMode_;
    itemBeingDownloaded = -1;
    pd = pd_;

    sortColumnOrder.append(COL_DATE); /* show first oldest files */
    for (int i = 0; i<NB_COLUMNS; i++) {
        sortIsAscending[i] = true;
    }
}

DownloadModel::~DownloadModel() {
    emptyFileList();
    if (DLTempFolder.size() > 0) {
        QDir(DLTempFolder).removeRecursively();
    }
}

void DownloadModel::sort(int column, Qt::SortOrder order){
    qDebug() << QString("sort(%1,%2)").arg(column).arg(order);
    sortColumnOrder.removeAll(column);
    sortColumnOrder.prepend(column);
    sortIsAscending[column] = (order == Qt::AscendingOrder);
    beginResetModel();
    sortSelection();
    endResetModel();
    reloaded();
}

void DownloadModel::sortSelection() {
    dpm_s = this;
    qSort(selectedFileList.begin(),selectedFileList.end(),fileLessThan);
    for (int i = 0; i < selectedFileList.size(); i++) {
        File *fi = selectedFileList.at(i);
        fi->modelRow = i;
    }
}

int DownloadModel::rowCount(const QModelIndex & parent __attribute__ ((unused))) const{
    return selectedFileList.size();
}
int DownloadModel::columnCount(const QModelIndex & parent __attribute__ ((unused))) const {
    return NB_COLUMNS;
}

QDateTime DownloadModel::dateForNewName(File* fi) const{
    if (dc->devices[id].toObject()[CONFIG_ALLOWEXIF].toBool() &&
            dc->devices[id].toObject()[CONFIG_USEEXIFDATE].toBool()    ) {
        return QDateTime::fromTime_t(fi->dateTaken());
    } else {
        return QDateTime::fromTime_t(fi->lastModified);
    }
}

QString DownloadModel::newPath(File *fi, bool keepDComStr) const{
    if (fi->parentFile != NULL) {
        QString parentNewPath = newPath(fi->parentFile);
        return parentNewPath.left(parentNewPath.lastIndexOf('.')) + "." + fi->fileName().split('.').last();
    } else {
        QJsonObject obj = dc->devices[id].toObject();
        QString newName = obj[CONFIG_NEWNAME].toString();
        QString newPath;
        QDateTime lm = dateForNewName(fi);
        int tagidx = -1;
        QStringList oNameList = fi->fileName().split(".");
        QString oExt = oNameList.last();
        oNameList.removeLast();
        QString oName = oNameList.join(".");

        for (int i = 0; i < dateKeywords.size(); i++) {
            newName.replace(dateKeywords.at(i),QLocale().toString(lm,dateKeywords.at(i)));
        }
        newName.replace("oName",oName);

        /* oFolder management */
        int oFolderPos = 0;
        QRegExp oFolderPattern = QRegExp("oFolder(-?)([1-9]\\d*)-(([1-9]\\d*)?)");
        QStringList oFolderList = fi->absoluteFilePath().mid(obj[CONFIG_PATH].toString().size()).split('/');
        oFolderList.removeLast();
        while ((oFolderPos = oFolderPattern.indexIn(newName,oFolderPos)) >= 0) {
            bool m_is_present;
            int n = oFolderPattern.cap(2).toInt();
            int m = oFolderPattern.cap(3).toInt(&m_is_present);
            bool invert = oFolderPattern.cap(1).size()>0;
            int first;
            int nb;
            if (m_is_present) {
                nb = std::max(n,m) - std::min(n,m) + 1;
                if (!invert) {
                    first = std::min(n,m) - 1;
                } else {
                    first = oFolderList.size() - std::max(n,m);
                }
            } else {
                if (!invert) {
                    first = n - 1;
                    nb = -1;
                } else {
                    first = 0;
                    nb = oFolderList.size() - (n - 1);
                    if (nb < 0) nb = 0;
                }
            }
            newName.replace(oFolderPattern.cap(),
                            QStringList(oFolderList.mid(first,nb)).join('/'));
        }
        oFolderPos = 0;
        oFolderPattern = QRegExp("oFolder(-?)([1-9]\\d*)");
        while ((oFolderPos = oFolderPattern.indexIn(newName,oFolderPos)) >= 0) {
            int n = oFolderPattern.cap(2).toInt();
            bool invert = oFolderPattern.cap(1).size()>0;
            int idx;
            if (!invert) {
                idx = n - 1;
            } else {
                idx = oFolderList.size() - n;
            }
            newName.replace(oFolderPattern.cap(),
                            (idx>=0 && idx<oFolderList.size())?oFolderList.at(idx):"");
        }

        if (!keepDComStr && newName.contains("dCom")) {
            newName.replace("dCom",getDCom(fi));
        }
        newName.replace("sCom",sessionComment);

        /* replace EXIF tags with their values */
        while (true) {
            tagidx = newName.indexOf("<",tagidx+1);
            if (tagidx<0) break;
            int endidx = newName.indexOf(">",tagidx);
            if (endidx<0) break;
            QString tagName = newName.mid(tagidx+1,endidx-tagidx-1);
            QString value = "";
            if (dc->devices[id].toObject()[CONFIG_ALLOWEXIF].toBool()) {
                value = fi->getEXIFValue(tagName);
            }
            newName.replace("<" + tagName + ">",value);
        }

        /* These are forbidden characters under windows: \ : * ? " < > | */
        newName.remove(QRegExp("[\\\\:*?\"<>|]"));

        /* prevent folder and file names to start or end with a space character */
        newName.prepend("/");
        while (newName.contains(" /")) {newName.replace(" /","/");}
        while (newName.contains("/ ")) {newName.replace("/ ","/");}

        newPath = obj[CONFIG_DOWNLOADTO].toString() + newName + "." + oExt;
        while (newPath.contains("//")) {newPath.replace("//","/");}

        { /* collisions management */
            int occurence_number = (newPathCollisions[newPath]++);
            if (occurence_number >= 1) {
                int insertPos = newPath.lastIndexOf('.');
                if (insertPos<0) insertPos = 0;
                newPath.insert(insertPos,QString("%1%2").arg((insertPos>0&&newPath[insertPos-1]!='/')?"-":"").arg(occurence_number));
            }
        }

        return newPath;
    }
}

QString &DownloadModel::newPath(File *fi) const{
    if (!newPathCache.contains(fi)) {
        newPathCache.insert(fi,newPath(fi,false));
    }
    return newPathCache[fi];
}

void DownloadModel::updateNewPaths() {
    newPathCache.clear();
    newPathCollisions.clear();
    dataChanged(createIndex(0,COL_NEWPATH),
                createIndex(selectedFileList.size()-1,COL_NEWPATH));
}

void DownloadModel::receiveGPSCoord(File *fi){
    qDebug() << "receiveGPSCoord " << fi->modelRow;
    dataChanged(createIndex(fi->modelRow,COL_GPS),
                createIndex(fi->modelRow,COL_GPS));
}

void DownloadModel::updateGPS(){
    qDebug() << "updateGPS";
    GPSRequested.clear();
    dataChanged(createIndex(0,COL_GPS),
                createIndex(selectedFileList.size()-1,COL_GPS));
}

/* Not used anymore */
QString DownloadModel::tempPath(File *fi) const {
    QStringList oNameList = fi->fileName().split(".");
    QString oExt = oNameList.last();
    QString tempName = QString(QCryptographicHash::hash(fi->absoluteFilePath().toUtf8(), QCryptographicHash::Sha3_224).toHex());
    return DLTempFolder + "/" + tempName + "." + oExt;
}

QString DownloadModel::guessCameraName() {
    if (dc->devices[id].toObject()[CONFIG_ALLOWEXIF].toBool()) {
        for (int i = selectedFileList.size()-1; i>=0; i--) {
            File * fi = selectedFileList.at(i);
            QString makeModel = fi->getEXIFValue("Make") + " " + fi->getEXIFValue("Model");
            if (makeModel.size() > 1) {
                return makeModel;
            }
        }
    }
    return "";
}

QString DownloadModel::getDCom(File *fi) const{
    QDateTime fileDate = QDateTime::fromTime_t(fi->lastModified);
    QString dayKey = fileDate.toString("yyyy-MM-dd");
    QJsonObject obj = dc->daily_comments;
    QJsonValue com = obj[dayKey];
    QString comment = com.toString();
    return comment;
}

bool DownloadModel::getAllCom(){
    qDebug() << "dpm.getAllCom";
    QString newName = dc->devices[id].toObject()[CONFIG_NEWNAME].toString();
    int res = QDialog::Accepted;
    if (selectedFileList.size() > 0 &&
        (newName.contains("dCom") ||
         newName.contains("sCom")   )  ) {
        QJsonObject obj = dc->daily_comments;
        DComDialog *allComQuery = new DComDialog();
        allComQuery->setWindowModality(Qt::WindowModal);
        QTableWidget *queryTable = allComQuery->ui->tableWidget;
        QMap<QString,QString> *daylycomments = new QMap<QString,QString>();
        int numberOfQueries = 0;
        queryTable->setColumnCount(2);
        if (newName.contains("dCom")) {
            for (int i = 0; i < selectedFileList.size(); i++) {
                File *fi = selectedFileList.at(i);
                QDate fileDate = dateForNewName(fi).date();
                QString dayKey = fileDate.toString("yyyy-MM-dd");
                QJsonValue com = obj[dayKey];
                if (!daylycomments->contains(dayKey)) {
                    QString comment = com.toString();
                    if (comment.size() == 0) {
                        /* Guess Comment based on existing arborescence at destination */
                        QString dirToSearch = "";
                        QString patternToSearch = "";
                        QStringList np = newPath(fi, true).split("/");
                        bool matchOnDirs = true;
                        int matchCount = 0;
                        int j;
                        for (j = 0; j< np.size(); j++) {
                            if (np.at(j).contains("dCom")) {
                                patternToSearch = np.at(j);
                                break;
                            } else {
                                if (dirToSearch.size()>0)
                                    dirToSearch.append("/");
                                dirToSearch.append(np.at(j));
                            }
                        }

                        if (j == np.size()-1) {
                            /* the dayly comment is in the file name -> match on files */
                            matchOnDirs = false;
                        }
                        qDebug() << "matchOnDirs=" << matchOnDirs;

                        QStringList entries = QDir(dirToSearch).entryList((matchOnDirs?(QDir::Dirs):(QDir::Files))|QDir::NoDotAndDotDot);
                        patternToSearch.replace("dCom","(.*)");
                        QRegExp pattern = QRegExp(patternToSearch);
                        qDebug() << "search " << dirToSearch << "(" << entries.size() << " entries) for" << patternToSearch;
                        for (int j = 0; j < entries.size(); j++) {
                            qDebug() << "   entry=" << entries.at(j);
                            int pos = pattern.indexIn(entries.at(j));
                            if (pos >= 0) {
                                qDebug() << "      match at " << pos;
                                matchCount++;
                                if (matchCount >= 4) {
                                    qDebug() << "         -> too many matches, abandon this meaningless search.";
                                    comment = "";
                                    break;
                                }
                                QString cap1 = pattern.cap(1);
                                if (cap1.size()>comment.size()) {
                                    comment = cap1;
                                }
                            }
                        }
                    }
                    daylycomments->insert(dayKey,comment);
                }
            }
            numberOfQueries += daylycomments->keys().size();
        }
        if (newName.contains("sCom")) {
            numberOfQueries++;
        }

        queryTable->setRowCount(numberOfQueries);
        for (int i = 0; i < daylycomments->keys().size(); i++) {
            QString dayKey = daylycomments->keys().at(i);
            QString dayComment = daylycomments->value(dayKey);
            QString dayName = QDateTime::fromString(dayKey,"yyyy-MM-dd").date().toString(Qt::SystemLocaleLongDate);
            QTableWidgetItem *keyItem = new QTableWidgetItem(dayName);
            keyItem->setFlags(Qt::ItemIsEnabled);
            QTableWidgetItem *valueItem = new QTableWidgetItem(dayComment);
            valueItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable );
            queryTable->setItem(i,0,keyItem);
            queryTable->setItem(i,1,valueItem);
        }
        if (newName.contains("sCom")) {
            QTableWidgetItem *keyItem = new QTableWidgetItem(tr("Session Comment"));
            keyItem->setFlags(Qt::ItemIsEnabled);
            QTableWidgetItem *valueItem = new QTableWidgetItem(sessionComment);
            valueItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
            queryTable->setItem(queryTable->rowCount()-1,0,keyItem);
            queryTable->setItem(queryTable->rowCount()-1,1,valueItem);
        }
        queryTable->resizeColumnsToContents();
        res = allComQuery->exec();
        for (int i = 0; i < daylycomments->keys().size(); i++) {
            obj[daylycomments->keys().at(i)] = queryTable->item(i,1)->text();
        }
        if (newName.contains("sCom")) {
            sessionComment = queryTable->item(queryTable->rowCount()-1,1)->text();
        }
        allComQuery->deleteLater();
        dc->daily_comments = obj;
        dc->saveDailyComments();

        updateNewPaths();
        delete daylycomments;
    }
    return (res == QDialog::Accepted);
}

void DownloadModel::showEXIFTags(int row) {
    File *fi = selectedFileList.at(row);
    QStringList tags = fi->getEXIFTags();
    if (tags.size() > 0) {
        EXIFDialog *d = new EXIFDialog();
        QTableWidget *t = d->ui->tableWidget;
        t->setRowCount(tags.size());
        for (int i = 0; i<tags.size(); i++) {
            QTableWidgetItem *keyItem = new QTableWidgetItem("<" + tags.at(i) + ">");
            keyItem->setFlags(Qt::ItemIsEnabled);
            QTableWidgetItem *valueItem = new QTableWidgetItem(fi->getEXIFValue(tags.at(i)));
            valueItem->setFlags(Qt::ItemIsEnabled);
            t->setItem(i,0,keyItem);
            t->setItem(i,1,valueItem);
        }
        d->exec();
        d->deleteLater();
    }

}


QVariant DownloadModel::data(const QModelIndex & index, int role) const{
    File *fi = selectedFileList.at(index.row());
    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case COL_THUMBNAIL:
            break;
        case COL_FILEPATH:
            return fi->absoluteFilePath();
            break;
        case COL_FILENAME:
            return fi->fileName();
            break;
        case COL_SIZE:
        {
            return File::size2Str(fi->size);
        }
            break;
        case COL_MODIFIED:
        {
            QDateTime lm = QDateTime::fromTime_t(fi->lastModified);
            return lm.toString("yyyy/MM/dd HH:mm:ss");
        }
            break;
        case COL_DATE:
        {
            QDateTime date = dateForNewName(fi);
            return date.toString("yyyy/MM/dd HH:mm:ss");
        }
            break;
        case COL_NEWPATH:
            if (excludedFiles.contains(fi)) {
                return tr("This file will not be downloaded.");
            } else {
                QString np = newPath(fi);
                if (np == fi->absoluteFilePath()) {
                    return tr("[same file]");
                } else {
                    return np;
                }
            }
            break;
        case COL_GPS:
            QJsonObject obj = dc->devices[id].toObject();
            if (!obj[CONFIG_GEOTAG].toBool()) {
                return "";
            }
            if (!GPSRequested.contains(fi)){
                GPSRequested.insert(fi);
                emit requestGPSCoord(fi);
            }
            if (fi->geotags.size() > 0 ) {
                QRegExp coord = QRegExp("(\\d+) (\\d+) (\\d*.?\\d*)");
                QString latiref = fi->geotags["GPSLatitudeRef"];
                QString longiref = fi->geotags["GPSLongitudeRef"];
                if ((latiref  == "N" || latiref  == "S") &&
                    (longiref == "E" || longiref == "W")   ) {
                    if (coord.indexIn(fi->geotags["GPSLatitude"]) >= 0) {
                        double lati = (coord.cap(1).toDouble() +
                                coord.cap(2).toDouble()/60 +
                                coord.cap(3).toDouble()/60/60) * (double)((latiref  == "N")?1:-1) ;
                        //qDebug() << lati << coord.cap() << coord.cap(1) << coord.cap(2) << coord.cap(3);
                        if (coord.indexIn(fi->geotags["GPSLongitude"]) >= 0) {
                            double longi = (coord.cap(1).toDouble() +
                                    coord.cap(2).toDouble()/60 +
                                    coord.cap(3).toDouble()/60/60) * (double)((longiref  == "E")?1:-1) ;
                            //qDebug() << longi << coord.cap() << coord.cap(1) << coord.cap(2) << coord.cap(3);
                            return QGeoCoordinate(lati,longi).toString();
                        }
                    }
                }
                /* raw format */
                return fi->geotags["GPSLatitudeRef"] + " "
                     + fi->geotags["GPSLatitude"] + " "
                     + fi->geotags["GPSLongitudeRef"] + " "
                     + fi->geotags["GPSLongitude"];
            } else {
                return "Computing GPS coordinates";
            }
            break;
        }
        break;
    case Qt::ToolTipRole:
        switch (index.column()) {
        case COL_THUMBNAIL:
        {
            QPixmap *tn = fi->getThumbnail();
            if (!tn->isNull()) {
                return QString("%1x%2").arg(tn->size().width()).arg(tn->size().height());
            }
        }
        case COL_FILEPATH:
        case COL_FILENAME:
        {
            QString tooltip = fi->absoluteFilePath();
            if (fi->parentFile != NULL) {
                tooltip += "\n" + QString(tr("This file is an attachment of %1")).arg(fi->parentFile->fileName());
            }
            if (fi->attachedFiles.size() != 0) {
                tooltip += "\n" + QString(tr("This file has %1 %2."))
                                    .arg(fi->attachedFiles.size())
                                    .arg(fi->attachedFiles.size()==1?tr("attachment"):tr("attachments"));
            }
            if (dc->knownFiles.contains(*fi)) {
                tooltip += "\n" + QString(tr("This file was tranfered before"));
            } else {
                tooltip += "\n" + QString(tr("This is a new file"));
            }
            return tooltip;
        }
            break;
        case COL_SIZE:
        {
            return QString(tr("%1 Bytes")).arg(fi->size);
        }
            break;
        case COL_MODIFIED:
        {
            QDateTime lm = QDateTime::fromTime_t(fi->lastModified);
            return lm.toString(Qt::SystemLocaleLongDate);
        }
            break;
        case COL_DATE:
        {
            QDateTime date = dateForNewName(fi);
            return date.toString(Qt::SystemLocaleLongDate);
        }
            break;
        case COL_NEWPATH:
        {
            if (!excludedFiles.contains(fi)) {
                QString tooltip = newPath(fi);
                QJsonObject obj = dc->devices[id].toObject();
                if (fi->transferOnGoing) {
                    tooltip += "\n" + tr("This file is being transfered");
                } else if (newPath(fi) == fi->absoluteFilePath()) {
                    if (obj[CONFIG_GEOTAG].toBool()) {
                        tooltip += "\n" + tr("Same file, it will just be geotagged");
                    } else {
                        tooltip += "\n" + tr("Same file, nothing will happen");
                    }
                } else if (QFile(newPath(fi)).exists()) {
                    if (fi->transferTo.size()>0) {
                        tooltip += "\n" + tr("This file was just transfered");
                    } else {
                        if (obj[CONFIG_OVERWRITEFILES].toBool()) {
                            tooltip += "\n" + tr("This file already exists but it WILL be overwritten");
                        } else {
                            tooltip += "\n" + tr("This file already exists and it will NOT be overwritten");
                        }
                    }
                } else {
                    if (obj[CONFIG_MOVEFILES].toBool()) {
                        tooltip += "\n" + tr("The file will be moved here");
                    } else {
                        tooltip += "\n" + tr("The file will be transfered here");
                    }
                }
                return tooltip;
            }
        }
            break;
        case COL_GPS:
        {
            if (fi->geotags.size() > 0 ) {
                QString gps = tr("These EXIF tags will be inserted in the file:") + "\n";
                QMapIterator<QString, QString> i(fi->geotags);
                while (i.hasNext()) {
                    i.next();
                    gps +=  i.key() + "=" + i.value() + "\n";
                }
                return gps;
            } else {
                return tr("The coordinates are being computed.");
            }
        }
            break;
        }
        break;
    case Qt::DecorationRole:
        if (index.column() == COL_THUMBNAIL) {
            QPixmap *tn = fi->getThumbnail();
            if (!tn->isNull()) {
                return *tn;
            } else {
                if (fi->isJPEG()) {
                    return QIcon(":/icons/picture");
                } else if (fi->isPicture()) {
                    return QIcon(":/icons/image");
                } else if (fi->isVideo()) {
                    return QIcon(":/icons/film");
                } else {
                    return QIcon(":/icons/file");
                }
            }
        }
        if (index.column() == COL_FILENAME ||
            index.column() == COL_FILEPATH) {
            if (dc->knownFiles.contains(*fi)) {
                return QIcon(":/icons/ok");
            } else {
                return QIcon(":/icons/add");
            }
        }
        if (index.column() == COL_NEWPATH) {
            QJsonObject obj = dc->devices[id].toObject();
            if (excludedFiles.contains(fi)) {
                return QIcon(":/icons/remove");
            } else if (fi->transferOnGoing) {
                return QIcon(":/icons/download");
            } else if (newPath(fi) == fi->absoluteFilePath()) {
                if (obj[CONFIG_GEOTAG].toBool()) {
                    return QIcon(":/icons/gps");
                } else {
                    return QIcon(":/icons/warning");
                }
            } else if (QFile(newPath(fi)).exists()) {
                if (fi->transferTo.size()>0) {
                    return QIcon(":/icons/ok");
                } else if (obj[CONFIG_OVERWRITEFILES].toBool()) {
                    return QIcon(":/icons/play");
                } else {
                    return QIcon(":/icons/warning");
                }
            } else {
                return QIcon(":/icons/play");
            }
        }
        if (index.column() == COL_GPS) {
            return QIcon(":/icons/gps");
        }
        break;
    case Qt::SizeHintRole:
        if (index.column() == COL_THUMBNAIL) {
            QPixmap *tn = fi->getThumbnail();
            if (!tn->isNull()) {
                return tn->size();
            }
        }
        break;
    }
    return QVariant();
}

Qt::ItemFlags DownloadModel::flags(const QModelIndex & index) const{
    switch (index.column()) {
    default:
        return Qt::ItemIsSelectable|
               Qt::ItemIsEnabled;
    }
}

QVariant DownloadModel::headerData(int section, Qt::Orientation orientation, int role) const{
    switch (role) {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
        switch (orientation) {
        case Qt::Horizontal:
            switch (section) {
            case COL_THUMBNAIL: return ""; break;
            case COL_FILEPATH: return tr("File Path"); break;
            case COL_FILENAME: return tr("File Name"); break;
            case COL_SIZE: return tr("Size"); break;
            case COL_MODIFIED: return tr("Date modified"); break;
            case COL_DATE: return tr("Date"); break;
            case COL_NEWPATH: return tr("Destination"); break;
            case COL_GPS: return tr("GPS Coordinates"); break;
            }
            break;
        case Qt::Vertical:
            break;
        }
    }
    return QVariant();
}

#define MIN_ROW_HEIGHT 30
QSize DownloadModel::thumbnailSize(int row) {
    File *fi =selectedFileList.at(row);

    QPixmap *tn = fi->getThumbnail();
    if (!tn->isNull()) {
        QSize size = tn->size();
        if (size.height() < MIN_ROW_HEIGHT) {
            size.setHeight(MIN_ROW_HEIGHT);
        }
        return size;
    } else {
        return QSize(MIN_ROW_HEIGHT,MIN_ROW_HEIGHT);
    }
}

bool pathLessThan(File *a, File *b) {
    return a->absoluteFilePath() < b->absoluteFilePath();
}
bool dateLessThan(File *a, File *b) {
    return a->lastModified < b->lastModified;
}


void DownloadModel::emptyFileList() {
    for (int i = 0; i < completeFileList.size(); i++) {
        delete completeFileList.at(i);
    }
    completeFileList.clear();
    selectedFileList.clear();
}

void DownloadModel::loadPreview(QString id_) {
    qDebug() << "dpm.loadPreview";
    id = id_;
    QJsonObject obj = dc->devices[id].toObject();
    sessionComment = "";
    QString path = obj[CONFIG_PATH].toString();
    QString IDPath = obj[CONFIG_IDPATH].toString();

    emptyFileList();

    discoveredFolders = 1;
    browsedFolders = 0;
    browsedFiles = 0;
    pdTimer.start();
    treatDir(File(0,path,0,true,IDPath));
    pd->hide();

    reloadSelection(true);
}


bool DownloadModel::isBlacklisted(File element) {
    QJsonObject obj = dc->devices[id].toObject();

    if (obj[CONFIG_LIMITDEPTH].toBool()){
        QStringList directoryList = element.absoluteFilePath().split("/");
        QString     root_of_search = obj[CONFIG_PATH].toString();
        int depth_of_root;
        int current_depth;
        int depth_limit = obj[CONFIG_DEPTHLIMIT].toInt();

        if (!element.isDir) {
            directoryList.removeLast();
        }
        current_depth = directoryList.size();

        while (root_of_search[root_of_search.size()-1] == '/') {
            root_of_search = root_of_search.left(root_of_search.size()-1);
        }
        depth_of_root = root_of_search.split('/').size();
        if (current_depth > depth_of_root + depth_limit) {
            return true;
        }
    }

    if (obj[CONFIG_FILTERTYPE].toInt() == 0) { /* from all directories */
        return false;
    } else {
        bool patternMatchedOnceInPath = false;
        int reverseNonMatchingIndex = 0;

        QStringList patternList = obj[CONFIG_FILTER].toString().split(";");
        QStringList directoryList = element.absoluteFilePath().split("/");
        QString     root_of_search = obj[CONFIG_PATH].toString();
        int depth_of_root;

        if (!element.isDir) {
            directoryList.removeLast();
        }

        while (root_of_search[root_of_search.size()-1] == '/') {
            root_of_search = root_of_search.left(root_of_search.size()-1);
        }
        depth_of_root = root_of_search.split('/').size();


        for (int dirIdx = directoryList.size()-1; dirIdx >= 0; dirIdx--) {
            bool patternMatchedOnce = false;
            for (int j = 0; j < patternList.size(); j++) {
                QString temp = patternList.at(j);
                QRegExp pattern = QRegExp("^" + temp.replace(".","\\.").replace("*",".*") + "$");
                if (pattern.indexIn(directoryList.at(dirIdx)) >= 0) {
                    patternMatchedOnce = true;
                    //qDebug() << element.absoluteFilePath() << "matched on" << patternList.at(j) << pattern;
                }
            }
            if (patternMatchedOnce) {
                patternMatchedOnceInPath = true;
            } else if (reverseNonMatchingIndex == 0) {
                reverseNonMatchingIndex = dirIdx;
            }
        }
        if (obj[CONFIG_FILTERTYPE].toInt() == 1) { /* from all directories except: */
            return patternMatchedOnceInPath;
        } else { /* from these directories: */
            int minMatchIdx = depth_of_root; /* Don't need to match H: in H:/DCIM/116_FUJI */
            if (element.IDPath.startsWith("WPD:/")) {
                /* Don't need to match "LGE Nexus 5/Mémoire de stockage interne"
                                     in LGE Nexus 5/Mémoire de stockage interne/DCIM/Camera/   */
                minMatchIdx = depth_of_root+1;
            }
            return reverseNonMatchingIndex >= minMatchIdx;
        }
    }
}

static bool bindFiles(File *file, File *attachment) {
    if (attachment->parentFile == NULL &&
            !attachment->isPicture() &&
            !attachment->isVideo() &&
            attachment->isAttachmentOf(file)) {
        attachment->parentFile = file;
        file->attachedFiles.append(attachment);
        /*qDebug() << QString("This file:            %1\nHas an attached file: %2")
                    .arg(file->absoluteFilePath())
                    .arg(attachment->absoluteFilePath());*/
        return true;
    } else {
        return false;
    }
}

void DownloadModel::treatDir(File dirInfo) {
    QList<File *> files;
    QList<File> directoriesToBrowse;
    bool theresMore = true;
    browsedFolders++;
    int estimatedTotalFiles = discoveredFolders*browsedFiles/browsedFolders;
    while (theresMore) {
        QApplication::processEvents();
        QList<File> contentPart = dirInfo.ls(&theresMore);
        for (int i = 0; i < contentPart.size(); ++i) {
            File element = contentPart.at(i);
            if (element.isDir) {
                discoveredFolders++;
                if (isBlacklisted(element)) {
                    qDebug() << "Do NOT list content of" << element.fileName();
                    blacklistedDirectories.append(element);
                } else {
                    directoriesToBrowse.append(element);
                }
            } else {
                browsedFiles++;
                files.append(new File(element));
            }
        }
        if (pdTimer.elapsed() > 2000 && !pd->isVisible()) {
            pd->reset();
            pd->setLabelText(tr("Looking for files on the device"));
            pd->setMinimum(0);
            pd->setMaximum(1);
            pd->setWindowModality(Qt::WindowModal);
            pd->setValue(0);
            pd->show();
        }
        if (pd->isVisible()) {
            if (browsedFiles>=estimatedTotalFiles) {
                estimatedTotalFiles = discoveredFolders*browsedFiles/browsedFolders;
            }
            pd->setMaximum(estimatedTotalFiles);
            pd->setValue(browsedFiles);
            if (pdTimer.elapsed() > 10000 &&
                    (dc->devices[id].toObject()[CONFIG_FILTER].toString().size() == 0 ||
                     dc->devices[id].toObject()[CONFIG_FILTERTYPE].toInt() == 0)) {
                pd->setLabelText(QString(tr("Your device is being browsed, and this is taking a long time...\n"\
                                            "Once this dialog window is finished, you will be able to setup\n"\
                                            "some filters on the directories that may or may not be browsed.\n"\
                                            "Doing so can dramatically speed-up this process for next time.")));
            }
        }
    }
    /* find attached files in the same directory */
    qSort(files.begin(),files.end(),pathLessThan);
    for (int i = 0; i < files.size(); ++i) {
        File *fi = files.at(i);
        if (fi->isPicture() || fi->isVideo()) {
            int j = i-1;
            while (j>=0) {
                File *af = files.at(j);
                if (bindFiles(fi,af)) {
                    j--;
                } else {
                    break;
                }
            }
            j = i+1;
            while (j<files.size()) {
                File *af = files.at(j);
                if (bindFiles(fi,af)) {
                    j++;
                } else {
                    break;
                }
            }
        }
    }
    completeFileList.append(files);

    for (int i = 0; i < directoriesToBrowse.size(); ++i) {
        treatDir(directoriesToBrowse.at(i));
    }
}

void DownloadModel::reloadSelection(bool firstTime) {
    qDebug() << "dpm.reloadSelection";
    QJsonObject obj = dc->devices[id].toObject();
    bool select_known       = (obj[CONFIG_FILESTODOWNLOAD].toString() == "All");
    bool select_pictures    = obj[CONFIG_PICTUREFILES].toBool();
    bool select_videos      = obj[CONFIG_VIDEOFILES].toBool();
    bool select_others      = obj[CONFIG_OTHERFILES].toBool();
    QString other_patterns  = obj[CONFIG_OTHERFILESPATTERNS].toString();
    beginResetModel();
    selectedFileList.clear();

    /* 1st step: check if some blacklisted directories need to be browsed */
    pdTimer.start();
    QMutableListIterator<File> bli(blacklistedDirectories);
    bool sortNeeded = false;
    while (bli.hasNext()) {
        File element = bli.next();
        if (!isBlacklisted(element)) {
            treatDir(element);
            bli.remove();
            sortNeeded = true;
        }
    }
    if (sortNeeded || firstTime) {
        qDebug() << "sort completeFileList by path";
        qSort(completeFileList.begin(), completeFileList.end(), pathLessThan);
        qDebug() << "copy completeFileList";
        completeFileList_byDate = completeFileList;
        qDebug() << "sort completeFileList by date";
        qSort(completeFileList_byDate.begin(), completeFileList_byDate.end(), dateLessThan);
    }


    qDebug() << "building selected file list by looping over " << completeFileList.size() << " files";

    for (int i = 0; i < completeFileList.size(); i++) {
        File *fi = completeFileList.at(i);
        if ((select_pictures && fi->isPicture()) ||
            (select_videos   && fi->isVideo  ()) ||
            (select_others   && fi->nameMatchesPatterns(other_patterns))){
            if ((select_known || !dc->knownFiles.contains(*fi)) &&
                !isBlacklisted(*fi)) {
                selectedFileList.append(fi);
                selectedFileList.append(fi->attachedFiles);
            }
        }
    }
    pd->hide();

    if (dc->devices[id].toObject()[CONFIG_ALLOWEXIF].toBool()) {
        qDebug() << "load exif tags";
        pdTimer.start();
        for (int i = 0; i < selectedFileList.size(); i++) {
            File *fi = selectedFileList.at(i);
            if (pd->isVisible()) {
                pd->setValue(i);
                if (pd->wasCanceled()) {
                    obj.insert(CONFIG_ALLOWEXIF,QJsonValue(false));
                    dc->devices[id] = obj;
                    dc->saveDevices();
                    EXIFLoadCanceled(false);
                    break;
                }
            }
            fi->loadExifData();
            if (pdTimer.elapsed() > 1000 && !pd->isVisible()) {
                /* only show the progress bar if the process lasts more than a second. */
                pd->reset();
                pd->setLabelText(tr("Loading EXIF Data"));
                pd->setMinimum(0);
                pd->setMaximum(selectedFileList.size());
                pd->setWindowModality(Qt::WindowModal);
                pd->setValue(0);
                qDebug() << "pd.show()";
                pd->show();
                qDebug() << "processEvents()";
                QApplication::processEvents();
                qDebug() << "processEvents() done";
            }
        }
        pd->hide();
        qDebug() << "pd.hide()";
    }
    qDebug() << "looping on selected files list";
    averagePicSize = 0;
    int countPics = 0;
    for (int i = 0; i < selectedFileList.size(); i++) {
        File *fi = selectedFileList.at(i);
        connect(fi,SIGNAL(readStarted(File*)),this,SLOT(readStarted(File*)),Qt::UniqueConnection);
        connect(fi,SIGNAL(writeFinished(File*)),this,SLOT(writeFinished(File*)),Qt::UniqueConnection);
        if (fi->isPicture()) {
            averagePicSize += fi->size;
            countPics++;
        }
    }
    if (countPics>0) {
        averagePicSize /= countPics;
    } else {
        for (int i = completeFileList.size()-1; i>=0 && countPics < 50; i--) {
            File *fi = completeFileList.at(i);
            if (fi->isPicture()) {
                averagePicSize += fi->size;
                countPics++;
            }
        }
        if (countPics > 0) {
            averagePicSize /= countPics;
        } else {
            averagePicSize = 5*1024*1024;
        }
    }
    sortSelection();
    qDebug() << "endResetModel";
    endResetModel();
    qDebug() << "reloaded";
    reloaded();
    qDebug() << "dpm.reloadSelection done";
}


void DownloadModel::readStarted(File * file){
    dataChanged(createIndex(file->modelRow,COL_NEWPATH),
                createIndex(file->modelRow,COL_NEWPATH));
    emit itemOfInterest(createIndex(file->modelRow,COL_NEWPATH));
}
void DownloadModel::writeFinished(File * file){
    dataChanged(createIndex(file->modelRow,COL_NEWPATH),
                createIndex(file->modelRow,COL_NEWPATH));
}


QString DownloadModel::getTrackingFolder() {
    QJsonObject obj = dc->devices[id].toObject();
    QString trackingFolder;
    if (obj[CONFIG_GEOTAG].toBool() == true) {
        if (obj[CONFIG_GEOTAGMODE].toString() == "OpenPaths.cc") {
            /* Download location information from OpenPath.cc */

            /* Convert downloaded JSON into GPX in temporary folder */

            trackingFolder = "";
        } else {
            trackingFolder = obj[CONFIG_TRACKFOLDER].toString();
        }
    } else {
        trackingFolder = "";
    }
    return trackingFolder;
}

void DownloadModel::getStats(qint64 *totalSize, int *nbFiles) {
    *totalSize = 0;
    for (int i = 0; i < selectedFileList.size(); i++) {
        File *fi = selectedFileList.at(i);
        if (!excludedFiles.contains(fi)) {
            *totalSize += fi->size;
            (*nbFiles)++;
        }
    }
}


void DownloadModel::freeUpSpace(bool isFakeRun,
                                qint64 *targetAvailable,
                                qint64 *bytesDeleted,
                                int *nbFilesDeleted){
    QJsonObject obj = dc->devices[id].toObject();
    qint64 totalSpace = obj[CONFIG_DEVICESIZE].toString().toLongLong();
    *targetAvailable = 0;
    *bytesDeleted = 0;
    *nbFilesDeleted = 0;
    deletedFiles.clear();

    if (obj[CONFIG_FREEUPSPACE].toBool() && totalSpace != 0) {
        qint64 available = obj[CONFIG_BYTESAVAILABLE].toString().toLongLong();
        bool protect_days = obj[CONFIG_PROTECTDAYS].toBool();
        bool protect_selection = obj[CONFIG_PROTECTTRANSFER].toBool();
        uint lastDate = QDateTime::currentDateTime().toTime_t();
        if (protect_days) {
            lastDate -= obj[CONFIG_PROTECTDAYSVALUE].toInt()*24*60*60;
            qDebug() << "Will not delete files modified after " << QDateTime::fromTime_t(lastDate).toString();
        }
        if (obj[CONFIG_TARGETNBPICS].toBool()) {
            qDebug() << "make space for " << obj[CONFIG_NBPICS].toString().toInt() << "NbFiles";
            *targetAvailable = std::max(*targetAvailable,obj[CONFIG_NBPICS].toString().toInt()*averagePicSize);
        }
        if (obj[CONFIG_TARGETPERCENTAGE].toBool()) {
            qDebug() << "target " << obj[CONFIG_TARGETPERCENTAGEVALUE].toInt() << "%";
            *targetAvailable = std::max(*targetAvailable,obj[CONFIG_TARGETPERCENTAGEVALUE].toInt()*totalSpace/100);
        }
        qDebug() << "completeFileList_byDate.size()=" << completeFileList_byDate.size();
        for (int i = 0;
             i < completeFileList_byDate.size() &&
             available + *bytesDeleted < *targetAvailable;
             i++) {
            File * fi = completeFileList_byDate.at(i);
            if (dc->knownFiles.contains(*fi)) {
                /* This file was transfered before */
                if (fi->lastModified < lastDate) {
                    if (protect_selection && selectedFileList.contains(fi)){
                        if (!isFakeRun)
                            qDebug() << "Do not delete this file as it was just downloaded:" << fi->fileName();
                    } else {
                        if (!isFakeRun) {
                            bool removed = fi->remove();
                            qDebug() << "Removing " << fi->absoluteFilePath() << " -> " << (removed?"OK":"KO") ;
                        }
                        *bytesDeleted += fi->size;
                        (*nbFilesDeleted)++;
                        deletedFiles.append(fi);
                    }

                } else {
                    if (!isFakeRun)
                        qDebug() << "Stop deleting because lastModified=" << QDateTime::fromTime_t(fi->lastModified).toString();
                    break;
                }
            } else {
                if (!isFakeRun)
                    qDebug() << "Do not delete unknown file: " << fi->absoluteFilePath();
            }
        }
    }
}


void DownloadModel::markForDownload(QModelIndexList rows, bool mark){
    for (int i = 0; i<rows.size(); i++) {
        int row = rows.at(i).row();
        File * fi = selectedFileList.at(row);
        if (mark) {
            excludedFiles.remove(fi);
        } else {
            excludedFiles.insert(fi);
        }
        dataChanged(createIndex(row,0),createIndex(row,NB_COLUMNS));
    }
    emit selectionModified();
}
























