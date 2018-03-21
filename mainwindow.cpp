#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "loggercategories.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	qInfo(logReportSaver()) << (QString("Successfully written spectrum for \"%1\"").arg(1)).toStdString().c_str();
}

MainWindow::~MainWindow()
{
	delete ui;
}
