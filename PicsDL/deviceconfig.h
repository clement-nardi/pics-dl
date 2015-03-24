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

#ifndef DEVICECONFIG_H
#define DEVICECONFIG_H

#include <QJsonObject>
#include "file.h"
#include <QSet>

#define FILE_LASTMODIFIED           "lastModified"
#define FILE_ABSOLUTEFILEPATH       "absoluteFilePath"
#define FILE_SIZE                   "size"

/* Drive properties */
#define CONFIG_ISMANAGED            "isManaged"
#define CONFIG_DISPLAYNAME          "displayName"
#define CONFIG_PATH                 "path"
#define CONFIG_IDPATH               "IDPath"
#define CONFIG_CAMERANAME           "CameraName"
#define CONFIG_DEVICESIZE           "DeviceSize"
#define CONFIG_BYTESAVAILABLE       "BytesAvailable"

/* macros for Download Tab */
#define CONFIG_FILESTODOWNLOAD      "FilesToDownLoad"
#define CONFIG_FILTERTYPE           "FilterType"
#define CONFIG_FILTER               "Filter"
#define CONFIG_DOWNLOADTO           "DownloadTo"
#define CONFIG_NEWNAME              "newName"
#define CONFIG_ALLOWEXIF            "AllowEXIF"
#define CONFIG_USEEXIFDATE          "UseEXIFDate"

/* macros for Geotag Tab */
#define CONFIG_GEOTAG               "GeoTag"
#define CONFIG_GEOTAGMODE           "GeoTagMode"
#define CONFIG_TRACKFOLDER          "TrackFolder"
#define CONFIG_OPACCESSKEY          "OPAccessKey"
#define CONFIG_OPSECRETKEY          "OPSecretKey"

/* macros for Free Up Space Tab */
#define CONFIG_FREEUPSPACE          "FreeUpSpace"
#define CONFIG_TARGETNBPICS         "TargetNbPics"
#define CONFIG_NBPICS               "NbPics"
#define CONFIG_TARGETPERCENTAGE     "TargetPercentage"
#define CONFIG_TARGETPERCENTAGEVALUE "TargetPercentageValue"
#define CONFIG_PROTECTDAYS          "ProtectDays"
#define CONFIG_PROTECTDAYSVALUE     "ProtectDaysValue"
#define CONFIG_PROTECTTRANSFER      "ProtectTransfer"

/* Automation box */
#define CONFIG_AUTOMATION           "automation"


class DeviceConfig {
public:
    explicit DeviceConfig();

    QJsonObject conf;
    QSet<File> knownFiles;

    void saveConfig();
    void loadConfig();
    void saveKnownFiles();
    void loadKnownFiles();

private:

    QString configFileName;
    QString knownFiles_FileName;

};

#endif // DEVICECONFIG_H
