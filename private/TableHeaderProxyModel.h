#ifndef IZSQLTABLEVIEW_TABLEHEADERPROXYMODEL_H
#define IZSQLTABLEVIEW_TABLEHEADERPROXYMODEL_H

#include <QSortFilterProxyModel>

namespace IzSQLTableView
{
	class TableHeaderModel;
	class SQLTableViewImpl;

	class TableHeaderProxyModel : public QSortFilterProxyModel
	{
		Q_OBJECT
		Q_DISABLE_COPY(TableHeaderProxyModel)

		// source model for this proxy
		Q_PROPERTY(TableHeaderModel* source READ source CONSTANT FINAL)

	public:
		// ctor
		explicit TableHeaderProxyModel(SQLTableViewImpl* tableView, QObject* parent);

		// returns internal m_sourceModel
		Q_INVOKABLE TableHeaderModel* source() const;

		// QAbstractProxyModel interface start

		void setSourceModel(QAbstractItemModel* sourceModel) override;

		// QAbstractProxyModel interface end

		// returns source row for given proxy column or -1 if invalid index was requested
		Q_INVOKABLE int sourceRow(int proxyRow) const;

		// returns source column for given proxy column or -1 if invalid index was requested
		Q_INVOKABLE int sourceColumn(int proxyColumn) const;

		// returns proxy row for given source row or -1 if invalid index was requested
		Q_INVOKABLE int proxyRow(int sourceRow) const;

		// returns proxy column for given source column or -1 if invalid index was requested
		Q_INVOKABLE int proxyColumn(int sourceColumn) const;

		// used to sort given column
		Q_INVOKABLE bool sortColumn(int column);

		// used to apply filter to a given column
		Q_INVOKABLE bool setColumnFilter(int column, const QString& filterValue);

		// used to set width for a given column
		Q_INVOKABLE bool setColumnWidth(int column, qreal columnWidth);

		// used to set visibility for given column
		Q_INVOKABLE bool setColumnVisibility(int column, bool visibility);

	private:
		// handler to the internal model
		TableHeaderModel* m_headerModel;

	protected:
		// QSortFilterProxyModel interface start

		bool filterAcceptsColumn(int source_column, const QModelIndex& source_parent) const override;

		// QSortFilterProxyModel interface end
	};

}   // namespace IzSQLTableView

#endif   // IZSQLTABLEVIEW_TABLEHEADERPROXYMODEL_H
