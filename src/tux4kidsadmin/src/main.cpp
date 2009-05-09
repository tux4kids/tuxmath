#include <QtGui/QApplication>

#include "mainWindow.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	MainController controller;
	MainWindow mainWindow(&controller);
	mainWindow.show();
	return app.exec();
}
