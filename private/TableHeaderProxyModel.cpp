#include "TableHeaderProxyModel.h"

#include <QDebug>

#include "TableHeaderModel.h"

IzSQLTableView::TableHeaderProxyModel::TableHeaderProxyModel(SQLTableViewImpl* tableView, QObject* parent)
	: QSortFilterProxyModel(parent)
	, m_headerModel(new TableHeaderModel(tableView, parent))
{
	// we don't really use sourceModel parameter in this function
	setSourceModel(nullptr);

	// model connects
	connect(m_headerModel, &TableHeaderModel::columnVisibilityChanged, this, &QSortFilterProxyModel::invalidate);
	connect(m_headerModel, &TableHeaderModel::excludedColumnsSet, this, &QSortFilterProxyModel::invalidate);
}

IzSQLTableView::TableHeaderModel* IzSQLTableView::TableHeaderProxyModel::source() const
{
	return m_headerModel;
}

void IzSQLTableView::TableHeaderProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
	Q_UNUSED(sourceModel)

	if (this->sourceModel() != nullptr) {
		qCritical() << "TableHeaderProxyModel automatically sets source model to the internal instance of TableHeaderModel. Overriding this functionality is not supported.";
	} else {
		QSortFilterProxyModel::setSourceModel(m_headerModel);
	}
}

int IzSQLTableView::TableHeaderProxyModel::sourceRow(int proxyRow) const
{
	return mapToSource(index(proxyRow, 0)).row();
}

int IzSQLTableView::TableHeaderProxyModel::sourceColumn(int proxyColumn) const
{
	return mapToSource(index(0, proxyColumn)).column();
}

int IzSQLTableView::TableHeaderProxyModel::proxyRow(int sourceRow) const
{
	return mapFromSource(m_headerModel->index(sourceRow, 0)).row();
}

int IzSQLTableView::TableHeaderProxyModel::proxyColumn(int sourceColumn) const
{
	return mapFromSource(m_headerModel->index(0, sourceColumn)).column();
}

bool IzSQLTableView::TableHeaderProxyModel::sortColumn(int column)
{
	return m_headerModel->sortColumn(sourceColumn(column));
}

bool IzSQLTableView::TableHeaderProxyModel::setColumnFilter(int column, const QString &filterValue)
{
	return m_headerModel->setColumnFilter(sourceColumn(column), filterValue);
}

bool IzSQLTableView::TableHeaderProxyModel::setColumnWidth(int column, qreal columnWidth)
{
	return m_headerModel->setColumnWidth(sourceColumn(column), columnWidth);
}

bool IzSQLTableView::TableHeaderProxyModel::setColumnVisibility(int column, bool visibility)
{
	return m_headerModel->setColumnVisibility(sourceColumn(column), visibility);
}

bool IzSQLTableView::TableHeaderProxyModel::filterAcceptsColumn(int source_column, const QModelIndex& source_parent) const
{
	Q_UNUSED(source_parent)

	return m_headerModel->data(m_headerModel->index(0, source_column), static_cast<int>(TableHeaderModel::TableHeaderModelRoles::IsVisible)).toBool();
}
