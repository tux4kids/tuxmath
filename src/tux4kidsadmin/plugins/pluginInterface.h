#ifndef PLUGIN_INTERFACE_H
#define PLUGIN_INTERFACE_H

#include <QString>
#include <QtPlugin>

class PluginInterface
{
public:
	virtual ~PluginInterface() {}

	virtual QString name() = 0;
};

Q_DECLARE_INTERFACE(PluginInterface, "org.Tux4Kids.Tux4KidsPluginInterface/0.0.1");

#endif
