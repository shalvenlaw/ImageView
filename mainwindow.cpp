#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    const cv::Mat image = cv::imread("./cat.jpg");
    ui->imageView->setMat(image);
}

MainWindow::~MainWindow()
{
    delete ui;
}

