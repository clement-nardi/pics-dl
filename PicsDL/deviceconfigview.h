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
#include <QCheckBox>
#include <QLineEdit>
#include <QSpinBox>

class TransferDialog;

namespace Ui {
class DeviceConfigView;
}

class BoxView {
public:
    BoxView(QCheckBox *check_box_, QString field_name_, bool default_value_);
    void setBox(QJsonObject *obj);
    void readBox(QJsonObject *obj);
    QCheckBox *check_box;
    QString field_name;
    bool default_value;
};

class LineEditView {
public:
    LineEditView(QLineEdit *line_edit_, QString field_name_, QString default_value_);
    void setLine(QJsonObject *obj);
    void readLine(QJsonObject *obj);
    QLineEdit *line_edit;
    QString field_name;
    QString default_value;
};

class SpinBoxView {
public:
    SpinBoxView(QSpinBox *spin_box_, QString field_name_, int default_value_);
    void setSpinBox(QJsonObject *obj);
    void readSpinBox(QJsonObject *obj);
    QSpinBox *spin_box;
    QString field_name;
    int default_value;
};

class DeviceConfigView : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceConfigView(DeviceConfig *dc, QString id, bool editMode = false, QWidget *parent = 0);
    ~DeviceConfigView();
    QString id;

private:
    bool editMode;
    void FillWithConfig();
    Ui::DeviceConfigView *ui;
    QList<BoxView> boxes;
    QList<LineEditView> lines;
    QList<SpinBoxView> spinBoxes;

    DeviceConfig *dc;
    QProgressDialog *pd;

    DownloadModel *dpm;
    TransferManager *tm;
    TransferDialog *td;
    bool geoTag();
    void showEvent(QShowEvent * event);

public slots:
    void chooseDLTo();
    void chooseTrackFolder();
    void CopyToConfig();
    void SaveConfig();
    void makeVisible(QModelIndex);
    void updateButton();
    void updateFreeUpSimulation();
    void showEXIFTags();
    void updateStatusText();
    void go();
    void postDownloadActions();

private slots:
    void handleLinks(QString link);
};

#endif // DEVICECONFIGVIEW_H
