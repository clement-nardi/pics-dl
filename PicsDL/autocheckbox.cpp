#include "autocheckbox.h"
#include <QStackedLayout>
#include <QTimer>

#define PB_MAX 1000
#define COUNT_DOWN 3000

autoCheckBox::autoCheckBox(QWidget *parent) :
    QWidget(parent)
{

    pb = new QProgressBar(this);
    pb->setMaximum(PB_MAX);
    pb->setValue(0);
    pb->setTextVisible(false);

    box = new QCheckBox("Perform this step\nautomatically next time",this);
    box->setStyleSheet("background:transparent");
    connect(box,SIGNAL(clicked()),this,SIGNAL(clicked()));

    QStackedLayout *sl = new QStackedLayout(this);
    sl->setStackingMode(QStackedLayout::StackAll);
    sl->addWidget(box);
    sl->addWidget(pb);

    setLayout(sl);

    countDown = new QTimer(this);
    countDown->setInterval(COUNT_DOWN);
    countDown->setSingleShot(true);
    connect(countDown,SIGNAL(timeout()),this, SLOT(treatCountDown()));

    refreshTimer = new QTimer(this);
    refreshTimer->setSingleShot(false);
    refreshTimer->setInterval(50);
    connect(refreshTimer,SIGNAL(timeout()),this, SLOT(refresh()));

}

autoCheckBox::~autoCheckBox()
{

}

void autoCheckBox::startCountDown() {
    if (box->isChecked()) {
        pb->setValue(PB_MAX);
        countDown->start();
        refreshTimer->start();
    }
}

void autoCheckBox::treatCountDown() {
    if (box->isChecked()) {
        performAction();
    }
}

void autoCheckBox::refresh() {
    int rt = countDown->remainingTime();
    if (rt > 0) {
        pb->setValue(rt*PB_MAX/COUNT_DOWN);
    } else {
        pb->setValue(0);
        refreshTimer->stop();
    }
}
