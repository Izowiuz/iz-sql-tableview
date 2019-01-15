#include "ColumnsSettingsWrapper.h"

#include "TableHeaderModel.h"

IzSQLTableView::ColumnsSettingsWrapper::ColumnsSettingsWrapper(TableHeaderModel* headerModel)
	: QAbstractItemModel(headerModel)
	, m_header(headerModel)
{
	// connects to the parent model
	connect(m_header, &TableHeaderModel::initialized, this, [this]() {
		beginResetModel();
		endResetModel();
	});

	connect(m_header, &TableHeaderModel::dataChanged, this, [this](auto& topLeft, auto& bottomRight, auto& roles) {
		emit dataChanged(index(topLeft.column(), 0), index(bottomRight.column(), 0), roles);
	});
}

QModelIndex IzSQLTableView::ColumnsSettingsWrapper::index(int row, int column, const QModelIndex& parent) const
{
	if (hasIndex(row, column, parent)) {
		return createIndex(row, column);
	}
	return {};
}

QModelIndex IzSQLTableView::ColumnsSettingsWrapper::parent(const QModelIndex& index) const
{
	return m_header->parent(index);
}

int IzSQLTableView::ColumnsSettingsWrapper::rowCount(const QModelIndex& parent) const
{
	return m_header->columnCount(parent);
}

int IzSQLTableView::ColumnsSettingsWrapper::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	return 1;
}

QVariant IzSQLTableView::ColumnsSettingsWrapper::data(const QModelIndex& index, int role) const
{
	return m_header->data(m_header->index(index.column(), index.row()), role);
}

bool IzSQLTableView::ColumnsSettingsWrapper::setData(int row, int column, const QVariant& value, const QString& role)
{
	return m_header->setData(row, column, value, role);
}

QHash<int, QByteArray> IzSQLTableView::ColumnsSettingsWrapper::roleNames() const
{
	return m_header->roleNames();
}
