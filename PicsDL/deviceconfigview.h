#ifndef DEVICECONFIGVIEW_H
#define DEVICECONFIGVIEW_H

#include <QWizard>
#include "deviceconfig.h"
#include "downloadmodel.h"
#include <QProgressDialog>

namespace Ui {
class DeviceConfigView;
}

class DeviceConfigView : public QWizard
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
    int nextId() const;
    bool validateCurrentPage();
    void initializePage(int id);

    DeviceConfig *dc;
    QProgressDialog *pd;
    DownloadModel *dpm;
    bool geoTag();
    void showEvent(QShowEvent * event);
public slots:
    void chooseDLTo();
    void chooseTrackFolder();
    void CopyToConfig();
    void SaveConfig();
    void makeVisible(QModelIndex);
    void updateButton();
    void showEXIFTags();
    void automation();
};

#endif // DEVICECONFIGVIEW_H
