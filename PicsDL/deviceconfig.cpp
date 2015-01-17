/**
 * Copyright 2014 Clément Nardi
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

#include "deviceconfig.h"
#include <QJsonDocument>
#include <QSettings>
#include <QDir>
#include <QDebug>
#include <QCoreApplication>
#include <QJsonArray>

DeviceConfig::DeviceConfig()
{
    QSettings settings(QSettings::IniFormat,
                       QSettings::UserScope,
                       QCoreApplication::organizationName(),
                       QCoreApplication::applicationName());

    QDir dir = QDir( QFileInfo(settings.fileName()).absolutePath() );
    qDebug() << dir.absolutePath();
    if (!dir.exists()) {
        dir.mkpath(dir.absolutePath());
    }
    configFileName = dir.absolutePath() + "/config.txt";
    qDebug() << configFileName;
    knownFiles_FileName = dir.absolutePath() + "/knownFiles.txt";
    qDebug() << knownFiles_FileName;

    loadConfig();
    loadKnownFiles();
}


void DeviceConfig::saveConfig(){
    QJsonDocument doc(conf);

    QFile configFile(configFileName);
    if (configFile.open(QIODevice::WriteOnly)) {
        configFile.write(doc.toJson());
        configFile.close();
    } else {
        qDebug() << "Could not save the config to file " << configFileName;
    }
    //qDebug() << QJsonDocument(conf).toJson();
}

void DeviceConfig::loadConfig(){

    QFile configFile(configFileName);
    if (configFile.open(QIODevice::ReadOnly)) {
        QJsonParseError error;
        conf = QJsonDocument::fromJson(configFile.readAll(), &error).object();
        if (error.error != QJsonParseError::NoError) {
            qDebug() << "Could not load the config from file " << configFileName << "error " << error.error << error.offset;
        } else {
            qDebug() << "Config Loaded";
        }
        configFile.close();
    } else {
        qDebug() << "Could not load the config from file " << configFileName;
    }
    qDebug() << QJsonDocument(conf).toJson();
}



void DeviceConfig::saveKnownFiles(){

    QFile knownFiles_File(knownFiles_FileName);
    if (knownFiles_File.open(QIODevice::WriteOnly)) {
        QJsonArray jsa;
        QSetIterator<File> i(knownFiles);
        int count = 0;
        while (i.hasNext()) {
            QJsonObject obj;
            File fi = i.next();
            obj["lastModified"] = QJsonValue(QString("%1").arg(fi.lastModified));
            obj["absoluteFilePath"] = QJsonValue(fi.absoluteFilePath);
            obj["size"] = QJsonValue(QString("%1").arg(fi.size));

            jsa.append(obj);
            count++;
        }

        QJsonDocument doc(jsa);
        knownFiles_File.write(doc.toJson());
        knownFiles_File.close();
        qDebug() << "properly saved " << count << " known files";
        //qDebug() << doc.toJson();
    } else {
        qDebug() << "Could not save the config to file " << configFileName;
    }
}

void DeviceConfig::loadKnownFiles(){
    QFile knownFiles_File(knownFiles_FileName);
    if (knownFiles_File.open(QIODevice::ReadOnly)) {
        QJsonParseError error;
        QJsonArray JKnownFiles;
        JKnownFiles = QJsonDocument::fromJson(knownFiles_File.readAll(), &error).array();
        if (error.error != QJsonParseError::NoError) {
            qDebug() << "Could not load the known files from file " << knownFiles_FileName << "error " << error.error << error.offset;
        } else {
            for (int i = 0; i < JKnownFiles.size(); i++) {
                QJsonObject kfo = JKnownFiles.at(i).toObject();
                knownFiles.insert(File(kfo["lastModified"].toString().toUInt(),
                                       kfo["absoluteFilePath"].toString(),
                                       kfo["size"].toString().toULongLong(),
                                       false));
            }
            qDebug() << JKnownFiles.size() << " known files Loaded";
        }
        knownFiles_File.close();
    } else {
        qDebug() << "Could not load the known files from file " << knownFiles_FileName;
    }
}
