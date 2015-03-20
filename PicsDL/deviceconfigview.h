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

#ifndef DEVICECONFIGVIEW_H
#define DEVICECONFIGVIEW_H

class DeviceConfig;
class DownloadModel;
class TransferManager;
#include <QProgressDialog>
#include <QWidget>
#include <QModelIndex>
#include <QTimer>
#include <QElapsedTimer>

namespace Ui {
class DeviceConfigView;
}

class DeviceConfigView : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceConfigView(DeviceConfig *dc, QString id, bool editMode = false, QWidget *parent = 0);
    ~DeviceConfigView();
    QString id;

private:
    bool editMode;
    void FillWithConfig(QString id);
    Ui::DeviceConfigView *ui;

    DeviceConfig *dc;
    QProgressDialog *pd;
    DownloadModel *dpm;
    TransferManager *tm;
    bool geoTag();
    void showEvent(QShowEvent * event);
    QTimer progressTimer;
    QElapsedTimer statsTimer;
    qint64 lastElapsed;
    qint64 lastTransfered;

public slots:
    void chooseDLTo();
    void chooseTrackFolder();
    void CopyToConfig();
    void SaveConfig();
    void makeVisible(QModelIndex);
    void updateButton();
    void showEXIFTags();
    void updateStatusText();
    void go();
    void updateProgress();
};

#endif // DEVICECONFIGVIEW_H
