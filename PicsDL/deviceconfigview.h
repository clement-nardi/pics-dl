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

class Config;
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
#include <QMenu>
#include <QAction>
#include <QGroupBox>

class TransferDialog;
class DeviceConfigView;

#define TAB_SELECT      0
#define TAB_ORGANIZE    1
#define TAB_GEOTAG      2
#define TAB_FREEUPSPACE 3
#define TAB_CUSTOM      4

namespace Ui {
class DeviceConfigView;
}

class BoxView: public QObject {
    Q_OBJECT
public:
    BoxView(QCheckBox *check_box_, QString field_name_, bool default_value_, DeviceConfigView *dcv_);
    void setBox();
    QCheckBox *check_box;
    QString field_name;
    bool default_value;
    DeviceConfigView *dcv;
private slots:
    void readBox();
};

class GroupBoxView: public QObject {
    Q_OBJECT
public:
    GroupBoxView(QGroupBox *group_box_, QString field_name_, bool default_value_, DeviceConfigView *dcv_);
    void setBox();
    QGroupBox *group_box;
    QString field_name;
    bool default_value;
    DeviceConfigView *dcv;
private slots:
    void readBox();
};

class LineEditView: public QObject {
    Q_OBJECT
public:
    LineEditView(QLineEdit *line_edit_, QString field_name_, QString default_value_, DeviceConfigView *dcv_);
private:
    void setLine();
    QLineEdit *line_edit;
    QString field_name;
    QString default_value;
    DeviceConfigView *dcv;
private slots:
    void readLine();
};

class SpinBoxView: public QObject {
    Q_OBJECT
public:
    SpinBoxView(QSpinBox *spin_box_, QString field_name_, int default_value_, DeviceConfigView *dcv_);
private:
    void setSpinBox();
    QSpinBox *spin_box;
    QString field_name;
    int default_value;
    DeviceConfigView *dcv;
private slots:
    void readSpinBox();
};

class DeviceConfigView : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceConfigView(Config *dc, QString id, bool editMode = false, QWidget *parent = 0);
    ~DeviceConfigView();
    Config *dc;
    QString id;

private:
    bool editMode;
    void FillWithConfig();
    void runCommandAsync(QString commandId);
    Ui::DeviceConfigView *ui;
    QList<BoxView* > checkBoxes;
    QList<GroupBoxView* > groupBoxes;
    QList<LineEditView*> lines;
    QList<SpinBoxView*> spinBoxes;

    DownloadModel *dpm;
    TransferManager *tm;
    TransferDialog *td;
    bool geoTag();
    void showEvent(QShowEvent * event);
    QTimer reloadTimer;
    QMenu tableMenu;
    QAction do_not_download_action;
    QAction download_action;

public slots:
    void chooseDLTo();
    void chooseTrackFolder();
    void readSpecialWidgets();
    void saveConfig();
    void makeVisible(QModelIndex);
    void updateButton();
    void updateFreeUpSimulation();
    void showEXIFTags();
    void updateStatusText();
    void go();
    void postDownloadActions();

private slots:
    void handleLinks(QString link);
    void resizeRows();
    void showTableContextMenu(QPoint p);
    void download_handle();
    void do_not_download_handle();
    void handleTabChange(int idx);
    void setNewNamePattern(QString pattern);
    void geotagToggled(bool checked);
    void init();

signals:
    void initLater();
};

#endif // DEVICECONFIGVIEW_H
