#include "progressbarlabel.h"
#include <QStackedLayout>

ProgressBarLabel::ProgressBarLabel(QWidget *parent) : QWidget(parent)
{
    bar = new QProgressBar(this);
    label = new QLabel(this);
    bar->setMinimumWidth(300);
    bar->setTextVisible(false);

    QStackedLayout * layout = new QStackedLayout(this);
    layout->setStackingMode(QStackedLayout::StackAll);
    layout->addWidget(label);
    layout->addWidget(bar);
    setLayout(layout);
    setVisible(false);
}

ProgressBarLabel::~ProgressBarLabel()
{

}

