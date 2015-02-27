#ifndef WPDIODEVICE_H
#define WPDIODEVICE_H

#include <QIODevice>

class WPDIODevice : public QIODevice
{
public:
    WPDIODevice(QString IDPath_);
    ~WPDIODevice();
    bool open(OpenMode mode);
    void close();
    bool atEnd() const;
    qint64 readData(char * data, qint64 maxSize);
    qint64 writeData(const char * data, qint64 maxSize);
    QString IDPath;
    bool at_end;
};

#endif // WPDIODEVICE_H
