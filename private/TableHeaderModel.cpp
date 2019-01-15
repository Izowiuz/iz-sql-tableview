#include "TableHeaderModel.h"

#include <QDebug>

#include "IzSQLUtilities/SQLTableModel.h"
#include "IzSQLUtilities/SQLTableProxyModel.h"

#include "ColumnsSettingsWrapper.h"
#include "SQLTableViewImpl.h"

IzSQLTableView::TableHeaderModel::TableHeaderModel(SQLTableViewImpl* tableView, QObject* parent)
	: QAbstractItemModel(parent)
	, m_tableView(tableView)
	, m_modelWrapper(new ColumnsSettingsWrapper(this))
{
	connect(m_tableView->model(), &IzSQLUtilities::SQLTableProxyModel::modelReset, this, [this]() {
		beginResetModel();
		m_columnProperties.clear();
		m_columnProperties.resize(this->columnCount());
		m_columnProperties.assign(m_columnProperties.capacity(),
								  { SortType::Unsorted,
									QString(),
									false,
									m_tableView->defaultColumnWidth(),
									true });
		endResetModel();
		emit initialized();
	});
}

QModelIndex IzSQLTableView::TableHeaderModel::index(int row, int column, const QModelIndex& parent) const
{
	if (hasIndex(row, column, parent)) {
		return createIndex(row, column);
	}
	return {};
}

int IzSQLTableView::TableHeaderModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	return 1;
}

int IzSQLTableView::TableHeaderModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	return m_tableView->model()->source()->columnCount();
}

QVariant IzSQLTableView::TableHeaderModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid()) {
		return {};
	}
	switch (role) {
	case Qt::DisplayRole: {
		return m_tableView->model()->source()->headerData(index.column(), Qt::Horizontal);
	}
	// SortType
	case Qt::UserRole: {
		return static_cast<int>(m_columnProperties.at(index.column()).sortType);
	}
	// IsFiltered
	case Qt::UserRole + 1: {
		return m_columnProperties.at(index.column()).isFiltered;
	}
	// FilterValue
	case Qt::UserRole + 2: {
		return m_columnProperties.at(index.column()).filterValue;
	}
	// ColumnWidth
	case Qt::UserRole + 3: {
		return m_columnProperties.at(index.column()).columnWidth;
	}
	// IsVisible
	case Qt::UserRole + 4: {
		return columnVisibility(index.column());
	}
	default:
		return {};
	}
	return {};
}

QModelIndex IzSQLTableView::TableHeaderModel::parent(const QModelIndex& child) const
{
	Q_UNUSED(child)
	return {};
}

QHash<int, QByteArray> IzSQLTableView::TableHeaderModel::roleNames() const
{
	m_cachedRoleNames.clear();
	m_reversedRoleNames.clear();
	m_cachedRoleNames.insert(static_cast<int>(TableHeaderModelRoles::DisplayData), QByteArrayLiteral("displayData"));
	m_cachedRoleNames.insert(static_cast<int>(TableHeaderModelRoles::SortType), QByteArrayLiteral("sortType"));
	m_cachedRoleNames.insert(static_cast<int>(TableHeaderModelRoles::IsFiltered), QByteArrayLiteral("isFiltered"));
	m_cachedRoleNames.insert(static_cast<int>(TableHeaderModelRoles::FilterValue), QByteArrayLiteral("filterValue"));
	m_cachedRoleNames.insert(static_cast<int>(TableHeaderModelRoles::ColumnWidth), QByteArrayLiteral("columnWidth"));
	m_cachedRoleNames.insert(static_cast<int>(TableHeaderModelRoles::IsVisible), QByteArrayLiteral("isVisible"));
	m_reversedRoleNames.insert(QStringLiteral("displayData"), static_cast<int>(TableHeaderModelRoles::DisplayData));
	m_reversedRoleNames.insert(QStringLiteral("sortType"), static_cast<int>(TableHeaderModelRoles::SortType));
	m_reversedRoleNames.insert(QStringLiteral("isFiltered"), static_cast<int>(TableHeaderModelRoles::IsFiltered));
	m_reversedRoleNames.insert(QStringLiteral("filterValue"), static_cast<int>(TableHeaderModelRoles::FilterValue));
	m_reversedRoleNames.insert(QStringLiteral("columnWidth"), static_cast<int>(TableHeaderModelRoles::ColumnWidth));
	m_reversedRoleNames.insert(QStringLiteral("isVisible"), static_cast<int>(TableHeaderModelRoles::IsVisible));

	return m_cachedRoleNames;
}

bool IzSQLTableView::TableHeaderModel::setData(int row, int column, const QVariant& value, const QString& role)
{
	return setData(index(row, column), value, m_reversedRoleNames[role]);
}

bool IzSQLTableView::TableHeaderModel::sortColumn(int column)
{
	if (columnIsValid(column)) {
		switch (m_columnProperties.at(column).sortType) {
		case SortType::Unsorted:
			return setData(index(0, column), static_cast<int>(SortType::Descending), static_cast<int>(TableHeaderModelRoles::SortType));
		case SortType::Ascending:
			return setData(index(0, column), static_cast<int>(SortType::Descending), static_cast<int>(TableHeaderModelRoles::SortType));
		case SortType::Descending:
			return setData(index(0, column), static_cast<int>(SortType::Ascending), static_cast<int>(TableHeaderModelRoles::SortType));
		}
	}
	qWarning() << "Column" << column << "is invalid.";
	return false;
}

bool IzSQLTableView::TableHeaderModel::setColumnFilter(int column, const QString& filterValue)
{
	if (columnIsValid(column)) {
		return setData(index(0, column), filterValue, static_cast<int>(TableHeaderModelRoles::FilterValue));
	}
	qWarning() << "Column" << column << "is invalid.";
	return false;
}

bool IzSQLTableView::TableHeaderModel::setColumnWidth(int column, qreal columnWidth)
{
	if (columnIsValid(column)) {
		if (columnWidth < 0) {
			qWarning() << "Column width" << columnWidth << "is invalid.";
			return false;
		}
		return setData(index(0, column), columnWidth, static_cast<int>(TableHeaderModelRoles::ColumnWidth));
	}
	qWarning() << "Column" << column << "is invalid.";
	return false;
}

qreal IzSQLTableView::TableHeaderModel::columnWidth(int column) const
{
	if (columnIsValid(column)) {
		return m_columnProperties.at(column).columnWidth == 0 ? m_tableView->defaultColumnWidth() : m_columnProperties.at(column).columnWidth;
	}
	qWarning() << "Column" << column << "is invalid.";
	return 0;
}

QString IzSQLTableView::TableHeaderModel::columnName(int column) const
{
	if (columnIsValid(column)) {
		return m_tableView->model()->sourceModel()->headerData(column, Qt::Horizontal).toString();
	}
	qWarning() << "Column" << column << "is invalid.";
	return {};
}

bool IzSQLTableView::TableHeaderModel::setColumnVisibility(int column, bool visibility)
{
	if (columnIsValid(column)) {
		return setData(index(0, column), visibility, static_cast<int>(TableHeaderModelRoles::IsVisible));
	}
	qWarning() << "Column" << column << "is invalid.";
	return false;
}

bool IzSQLTableView::TableHeaderModel::columnVisibility(int column) const
{
	if (columnIsValid(column)) {
		return m_columnProperties.at(column).isVisible && !m_excludedColumns.contains(column);
	}
	qWarning() << "Column" << column << "is invalid.";
	return false;
}

bool IzSQLTableView::TableHeaderModel::columnIsValid(int column) const
{
	return column >= 0 && column < static_cast<int>(m_columnProperties.size());
}

IzSQLTableView::ColumnsSettingsWrapper* IzSQLTableView::TableHeaderModel::modelWrapper() const
{
	return m_modelWrapper;
}

void IzSQLTableView::TableHeaderModel::initializeColumnWidth(int column, qreal columnWidth)
{
	if (columnIsValid(column)) {
		if (columnWidth < 0) {
			qWarning() << "Column width" << columnWidth << "is invalid.";
			return;
		}
		m_columnProperties.at(column).columnWidth = columnWidth;
		return;
	}
	qWarning() << "Column" << column << "is invalid.";
}

bool IzSQLTableView::TableHeaderModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (!index.isValid()) {
		qWarning() << "Index is invalid.";
		return false;
	}
	switch (role) {
	// SortType
	case Qt::UserRole: {
		if (m_columnProperties.at(index.column()).sortType != static_cast<SortType>(value.toInt())) {
			for (int i = 0; i < static_cast<int>(m_columnProperties.size()); ++i) {
				if (i == index.column()) {
					m_columnProperties.at(i).sortType = static_cast<SortType>(value.toInt());
				} else {
					m_columnProperties.at(i).sortType = SortType::Unsorted;
				}
			}
			emit columnSorted(index.column(), static_cast<SortType>(value.toInt()));
			emit dataChanged(this->index(0, 0), this->index(0, columnCount() - 1), { static_cast<int>(TableHeaderModelRoles::SortType) });
			return true;
		}
		return false;
	}
	// FilterValue
	case Qt::UserRole + 2: {
		if (m_columnProperties.at(index.column()).filterValue != value.toString()) {
			m_columnProperties.at(index.column()).filterValue = value.toString();
			m_columnProperties.at(index.column()).isFiltered  = !m_columnProperties.at(index.column()).filterValue.isEmpty();
			emit filterChanged(index.column(), value.toString());
			emit dataChanged(index, index, { static_cast<int>(TableHeaderModelRoles::IsFiltered), static_cast<int>(TableHeaderModelRoles::FilterValue) });
			return true;
		}
		return false;
	}
	// ColumnWidth
	case Qt::UserRole + 3: {
		if (m_columnProperties.at(index.column()).columnWidth != value.toReal()) {
			m_columnProperties.at(index.column()).columnWidth = value.toReal();
			emit columnWidthChanged(index.column(), value.toReal());
			emit dataChanged(index, index, { static_cast<int>(TableHeaderModelRoles::ColumnWidth) });
			return true;
		}
		return false;
	}
	// IsVisible
	case Qt::UserRole + 4: {
		if (m_columnProperties.at(index.column()).isVisible != value.toBool() && !m_excludedColumns.contains(index.column())) {
			m_columnProperties.at(index.column()).isVisible = value.toBool();
			emit columnVisibilityChanged(index.column(), value.toBool());
			emit dataChanged(index, index, { static_cast<int>(TableHeaderModelRoles::IsVisible) });
			return true;
		}
		return false;
	}
	default:
		return false;
	}
	return false;
}

QSet<int> IzSQLTableView::TableHeaderModel::excludedColumns() const
{
	return m_excludedColumns;
}

void IzSQLTableView::TableHeaderModel::setExcludedColumns(const QSet<int>& excludedColumns)
{
	if (m_excludedColumns != excludedColumns) {
		m_excludedColumns = excludedColumns;
		emit dataChanged(index(0, 0), index(0, columnCount() - 1), { static_cast<int>(TableHeaderModelRoles::IsVisible) });
		emit excludedColumnsSet();
	}
}
