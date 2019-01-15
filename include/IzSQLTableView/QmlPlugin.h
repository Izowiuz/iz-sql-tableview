#ifndef IZSQLTABLEVIEW_QMLPLUGIN_H
#define IZSQLTABLEVIEW_QMLPLUGIN_H

#include <QQmlExtensionPlugin>

namespace IzSQLTableView
{
	class QmlPlugin : public QQmlExtensionPlugin
	{
		Q_OBJECT
		// WARNING: bez CMake można skorzystać z DEFINE: QQmlExtensionInterface_iid
		Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface/1.0")

	public:
		// QQmlTypesExtensionInterface interface
		void registerTypes(const char* uri) override;
	};
}   // namespace IzSQLTableView

#endif   // IZSQLTABLEVIEW_QMLPLUGIN_H
