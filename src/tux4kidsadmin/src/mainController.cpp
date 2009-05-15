#include <QDebug>
#include <QDir>
#include <QApplication>
#include <QPluginLoader>

#include "mainController.h"

MainController::MainController()
{
	loadPlugins();

	if (plugins.isEmpty()) {
		qDebug() << tr("Failed to load plugins");
	} else {
		qDebug() << tr("Loaded plugins: ");
		foreach(PluginInterface *plugin, plugins) {
			qDebug() << plugin->name();
		}
	}
}

MainController::~MainController()
{
}

void MainController::loadPlugins()
{
	PluginInterface *pluginInterface;
	QDir pluginsDir(qApp->applicationDirPath() + "/plugins");

	foreach (QString fileName, pluginsDir.entryList(QDir::Files))
	{
		QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName));
		QObject *plugin = pluginLoader.instance();
		if (plugin)
		{
			pluginInterface = qobject_cast<PluginInterface *>(plugin);
			if (pluginInterface)
			{
				plugins.append(pluginInterface);
			}
		}
	}
}

