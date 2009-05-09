#include "dummy.h"

DummyPlugin::DummyPlugin(QObject *parent) : QObject(parent)
{
}

DummyPlugin::~DummyPlugin()
{
}

QString DummyPlugin::name()
{
	return "dummy plugin";
}

Q_EXPORT_PLUGIN2(dummyPlugin, DummyPlugin);
