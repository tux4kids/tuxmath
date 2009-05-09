#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <QList>

#include "pluginInterface.h"

class MainController : public QObject
{
	Q_OBJECT

public:
	MainController();
	~MainController();

private:
	QList<PluginInterface *> plugins;

	void loadPlugins();
};

#endif // MAINCONTROLLER_H
