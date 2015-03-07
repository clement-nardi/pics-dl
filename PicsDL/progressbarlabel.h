#ifndef PROGRESSBARLABEL_H
#define PROGRESSBARLABEL_H

#include <QWidget>
#include <QProgressBar>
#include <QLabel>

class ProgressBarLabel : public QWidget
{
    Q_OBJECT
public:
    explicit ProgressBarLabel(QWidget *parent = 0);
    ~ProgressBarLabel();

    QProgressBar *bar;
    QLabel *label;

signals:

public slots:
};

#endif // PROGRESSBARLABEL_H
