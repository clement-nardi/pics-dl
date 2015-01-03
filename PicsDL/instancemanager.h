#ifndef INSTANCEMANAGER_H
#define INSTANCEMANAGER_H

#include <QThread>
#include <QSystemSemaphore>
#include <QSharedMemory>

class InstanceManager : public QThread
{
    Q_OBJECT
public:
    explicit InstanceManager(QObject *parent = 0);
    ~InstanceManager();
    void run();
    bool isFirstInstance;

signals:
    void applicationLaunched();

public slots:

private:
    QSystemSemaphore *sem;
    QSharedMemory *sm;
};

#endif // INSTANCEMANAGER_H
