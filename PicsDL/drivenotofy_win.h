#ifndef DRIVENOTOFY_WIN_H
#define DRIVENOTOFY_WIN_H

#include <shlobj.h>
#include <shlwapi.h>
#include <QWidget>

class DriveNotofy_win : public QWidget
{
    Q_OBJECT
public:
    explicit DriveNotofy_win(QWidget *parent = 0);
    ~DriveNotofy_win();
protected:
    bool nativeEvent(const QByteArray & eventType, void * message, long * result);
private:
    static const unsigned int msgShellChange = WM_USER + 1;
    unsigned long id;
    static QString getPidlPath(ITEMIDLIST* pidl);
signals:
    void driveAdded(QString);
};

#endif // DRIVENOTOFY_WIN_H

