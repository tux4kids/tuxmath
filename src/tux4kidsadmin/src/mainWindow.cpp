#include "mainWindow.h"
#include "ui_mainWindow.h"

MainWindow::MainWindow(MainController *controller, QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	mainController = controller;
}

MainWindow::~MainWindow()
{
	delete ui;
}
