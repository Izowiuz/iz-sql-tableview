#include "IzSQLTableView/QmlPlugin.h"

#include <QtQml>

#include "IzSQLUtilities/SQLTableModel.h"
#include "IzSQLUtilities/SQLTableProxyModel.h"

#include "ColumnsSettingsWrapper.h"
#include "SQLTableViewImpl.h"
#include "TableHeaderModel.h"
#include "TableHeaderProxyModel.h"

void IzSQLTableView::QmlPlugin::registerTypes(const char* uri)
{
	Q_ASSERT(uri == QLatin1String("IzSQLTableView"));

	qRegisterMetaType<TableHeaderModel*>("TableHeaderModel*");
	qRegisterMetaType<TableHeaderProxyModel*>("TableHeaderProxyModel*");
	qRegisterMetaType<ColumnsSettingsWrapper*>("ColumnsSettingsWrapper*");
	qRegisterMetaType<IzSQLUtilities::SQLTableModel*>("SQLTableModel*");
	qRegisterMetaType<IzSQLUtilities::SQLTableProxyModel*>("SQLTableProxyModel*");
	qRegisterMetaType<QItemSelectionModel::SelectionFlags>("QItemSelectionModel::SelectionFlags");
	qRegisterMetaType<IzSQLTableView::TableSelectionMode>("IzSQLTableView::TableSelectionMode");

	qmlRegisterType<IzSQLUtilities::SQLTableModel>(uri, 1, 0, "IzSQLTableModel");
	qmlRegisterType<IzSQLTableView::SQLTableViewImpl>(uri, 1, 0, "SQLTableViewImpl");
	qmlRegisterType(QUrl(QStringLiteral("qrc:/include/IzSQLTableView/QML/IzSQLTableView.qml")), "IzSQLTableView", 1, 0, "IzSQLTableView");

	qmlRegisterUncreatableMetaObject(IzSQLTableView::staticMetaObject,
									 uri,
									 1,
									 0,
									 "IzSQLTableViewEnums",
									 QStringLiteral("Error: only enums"));
}
