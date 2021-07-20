#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include "temperature.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    init_temp_sensor();
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(on_pushButton_clicked()));
    timer->start(5000);
}

MainWindow::~MainWindow()
{
    close_temp();
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    ui->lcdNumber->display((float)((float)bmp280_get_temp()/100));
    ui->lcdNumber_2->display(bmp280_get_pressure());
}
