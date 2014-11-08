#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QGridLayout>
#include <QJsonObject>
#include <QLabel>
#include "driveview.h"
#include <QMessageBox>
#include "verticalscrollarea.h"


MainWindow::MainWindow(DeviceConfig *dc_, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    dc = dc_;
    ui->setupUi(this);

    QMenu *menu = new QMenu;
    menu->addAction(ui->actionShow);
    menu->addAction(ui->actionQuit);
    menu->setDefaultAction(ui->actionShow);
    sysTray.setContextMenu(menu);
    sysTray.setIcon(QIcon(":/icons/picsDL"));
    sysTray.show();

    connect(&sysTray,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(sysTray_handle(QSystemTrayIcon::ActivationReason)));
    connect(ui->actionQuit, SIGNAL(triggered()),this,SLOT(quit_handle()));
    connect(ui->actionShow, SIGNAL(triggered()),this,SLOT(show_handle()));

    row = 0;
    load();
}

void MainWindow::sysTray_handle(QSystemTrayIcon::ActivationReason reason) {
    switch(reason) {
    case QSystemTrayIcon::DoubleClick:
    case QSystemTrayIcon::Trigger:
        sysTray.contextMenu()->defaultAction()->trigger();
        break;
    }
}

void MainWindow::show_handle() {
    show();
}

void MainWindow::quit_handle() {
    QMessageBox msgBox;
    msgBox.setText(QString(tr("Are you sure you want to exit PicsDL?")));
    msgBox.setInformativeText(tr("PicsDL needs to run in the background in order to detect when you plug a camera or memory card to your computer."));
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int ret = msgBox.exec();

    if (ret==QMessageBox::Yes) {
        QApplication::exit(0);
    }
}

void MainWindow::load() {
    VerticalScrollArea *sa = new VerticalScrollArea(this);
    QWidget *driveList = new QWidget(this);

    gl = new QGridLayout();

    QJsonObject::iterator i;
    for (i = dc->conf.begin(); i != dc->conf.end(); ++i) {
        insertDrive(i.key());
    }
    gl->setRowStretch(10000,1);
    gl->setColumnStretch(1,1);
    gl->setSizeConstraint(QLayout::SetMinAndMaxSize);
    driveList->setLayout(gl);

    sa->setWidgetResizable(true);
    sa->setWidget(driveList);
    setCentralWidget(sa);
}


void MainWindow::insertDrive(QString id){
    int col;
    DriveView *dv = new DriveView(id,dc);
    col = 0;
    gl->setRowMinimumHeight(row,30);
    gl->addWidget(dv->driveIcon,row,col++);
    gl->addWidget(dv->label,row,col++);
    gl->addWidget(dv->managedBox,row,col++);
    gl->addWidget(dv->editButton,row,col++);
    gl->addWidget(dv->removeButton,row,col++);
    connect(dv,SIGNAL(destroyed(QObject*)),this,SLOT(removeDrive(QObject*)));
    dvl.insert(dv);
    row++;
}
void MainWindow::removeDrive(QObject * dv){
    dvl.remove((DriveView *)dv);
}

void MainWindow::showEvent(QShowEvent * event){
    QSetIterator<DriveView *> i(dvl);
    while (i.hasNext()) {
        DriveView * dv = i.next();
        dv->loadName();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}
