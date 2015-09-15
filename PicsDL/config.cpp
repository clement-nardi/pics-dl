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

#include "config.h"
#include <QJsonDocument>
#include <QSettings>
#include <QDir>
#include <QDebug>
#include <QCoreApplication>
#include <QJsonArray>
#include <QWidget>

Config::Config()
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

    devices_file_name = dir.absolutePath() + "/devices.txt";
    qDebug() << devices_file_name;
    known_files_file_name = dir.absolutePath() + "/knownFiles.txt";
    qDebug() << known_files_file_name;
    daily_comments_file_name = dir.absolutePath() + "/dailyComments.txt";
    qDebug() << daily_comments_file_name;
    gui_params_file_name = dir.absolutePath() + "/GUIParams.txt";
    qDebug() << gui_params_file_name;

    /* migrate config.txt (< v0.4.1) to devices.txt */
    QFile config(dir.absolutePath() + "/config.txt");
    if (config.exists()) {
        qDebug() << "migrating configuration file for version > 0.4.1";
        config.rename(devices_file_name);
    }

    loadKnownFiles();
    loadDevices();
    loadDailyComments();
    loadGUIParams();
}

void Config::saveDevices(){
    saveJSONToFile(&devices,devices_file_name);
}

void Config::saveDailyComments(){
    saveJSONToFile(&daily_comments,daily_comments_file_name);
}

void Config::saveGUIParams(){
    saveJSONToFile(&gui_params,gui_params_file_name);
}


void Config::saveWindowGeometry(QWidget *w, QString key){
    QJsonObject obj = gui_params[key].toObject();
    obj.insert(WINDOW_X, QJsonValue(w->geometry().x()));
    obj.insert(WINDOW_Y, QJsonValue(w->geometry().y()));
    obj.insert(WINDOW_WIDTH, QJsonValue(w->geometry().width()));
    obj.insert(WINDOW_HEIGHT, QJsonValue(w->geometry().height()));
    obj.insert(WINDOW_IS_MAXIMIZED, QJsonValue(w->isMaximized()));
    gui_params.insert(key,obj);
    saveGUIParams();
}

bool Config::LoadWindowGeometry(QString key, QWidget *w){
    QJsonObject obj = gui_params[key].toObject();
    if (obj.contains(WINDOW_WIDTH)) {
        w->setGeometry(obj.value(WINDOW_X).toInt(),
                       obj.value(WINDOW_Y).toInt(),
                       obj.value(WINDOW_WIDTH).toInt(),
                       obj.value(WINDOW_HEIGHT).toInt() );
        w->setWindowState(obj.value(WINDOW_IS_MAXIMIZED).toBool()?(Qt::WindowMaximized):(Qt::WindowNoState));
        return true;
    }
    return false;
}


void Config::loadDevices(){
    loadJSONFromFile(&devices,devices_file_name);
}

void Config::loadDailyComments(){
    loadJSONFromFile(&daily_comments,daily_comments_file_name);
}

void Config::loadGUIParams(){
    loadJSONFromFile(&gui_params,gui_params_file_name);
}



void Config::saveJSONToFile(QJsonObject *obj, QString file_name){
    QJsonDocument doc(*obj);

    QFile dest_file(file_name);
    if (dest_file.open(QIODevice::WriteOnly)) {
        dest_file.write(doc.toJson());
        dest_file.close();
    } else {
        qDebug() << "Could not save the config to file " << file_name;
    }
    //qDebug() << QJsonDocument(obj).toJson();
}

void Config::loadJSONFromFile(QJsonObject *obj, QString file_name){

    QFile json_file(file_name);
    if (json_file.open(QIODevice::ReadOnly)) {
        QJsonParseError error;
        *obj = QJsonDocument::fromJson(json_file.readAll(), &error).object();
        if (error.error != QJsonParseError::NoError) {
            qDebug() << "Could not load the JSON data from file " << file_name << "error " << error.error << error.offset;
        } else {
            qDebug() << "JSON data Loaded from " << file_name;
        }
        json_file.close();
    } else {
        qDebug() << "Could not open the file containing JSON data: " << file_name;
    }
    //qDebug() << QJsonDocument(*obj).toJson();
}



void Config::saveKnownFiles(){

    QFile knownFiles_File(known_files_file_name);
    if (knownFiles_File.open(QIODevice::WriteOnly)) {
        QJsonArray jsa;
        QSetIterator<File> i(knownFiles);
        int count = 0;
        while (i.hasNext()) {
            QJsonObject obj;
            File fi = i.next();
            obj[FILE_LASTMODIFIED] = QJsonValue(QString("%1").arg(fi.lastModified));
            obj[FILE_ABSOLUTEFILEPATH] = QJsonValue(fi.absoluteFilePath());
            obj[FILE_SIZE] = QJsonValue(QString("%1").arg(fi.size));

            jsa.append(obj);
            count++;
        }

        QJsonDocument doc(jsa);
        knownFiles_File.write(doc.toJson());
        knownFiles_File.close();
        qDebug() << "properly saved " << count << " known files";
        //qDebug() << doc.toJson();
    } else {
        qDebug() << "Could not save the config to file " << devices_file_name;
    }
}

void Config::loadKnownFiles(){
    QFile knownFiles_File(known_files_file_name);
    if (knownFiles_File.open(QIODevice::ReadOnly)) {
        QJsonParseError error;
        QJsonArray JKnownFiles;
        JKnownFiles = QJsonDocument::fromJson(knownFiles_File.readAll(), &error).array();
        if (error.error != QJsonParseError::NoError) {
            qDebug() << "Could not load the known files from file " << known_files_file_name << "error " << error.error << error.offset;
        } else {
            for (int i = 0; i < JKnownFiles.size(); i++) {
                QJsonObject kfo = JKnownFiles.at(i).toObject();
                knownFiles.insert(File(kfo[FILE_LASTMODIFIED].toString().toUInt(),
                                       kfo[FILE_ABSOLUTEFILEPATH].toString(),
                                       kfo[FILE_SIZE].toString().toULongLong(),
                                       false));
            }
            qDebug() << JKnownFiles.size() << " known files Loaded";
        }
        knownFiles_File.close();
    } else {
        qDebug() << "Could not load the known files from file " << known_files_file_name;
    }
}


void Config::deviceFieldChanged(QString id){
    emit configStructuralChange(id);
}
