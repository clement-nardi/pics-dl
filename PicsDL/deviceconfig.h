#ifndef DEVICECONFIG_H
#define DEVICECONFIG_H

#include <QJsonObject>
#include "fileinfo.h"
#include <QSet>

class DeviceConfig {
public:
    explicit DeviceConfig();

    QJsonObject conf;
    QSet<FileInfo> *knownFiles;

    void saveConfig();
    void loadConfig();
    void saveKnownFiles();
    void loadKnownFiles();

private:

    QString configFileName;
    QString knownFiles_FileName;

};

#endif // DEVICECONFIG_H
