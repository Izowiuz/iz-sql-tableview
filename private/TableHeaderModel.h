#ifndef IZSQLTABLEVIEW_TABLEHEADERMODEL_H
#define IZSQLTABLEVIEW_TABLEHEADERMODEL_H

#include <vector>

#include <QAbstractItemModel>

#include "IzSQLTableView/IzSQLTableViewPlugin_Enums.h"

namespace IzSQLTableView
{
	class SQLTableViewImpl;
	class ColumnsSettingsWrapper;

	// TODO: do ColumnProperties dodać nazwę kolumny
	class TableHeaderModel : public QAbstractItemModel
	{
		Q_OBJECT
		Q_DISABLE_COPY(TableHeaderModel)

		// properties of the single column
		struct ColumnProperties {
			SortType sortType{ SortType::Unsorted };
			QString filterValue;
			bool isFiltered{ false };
			qreal columnWidth{ 120 };
			bool isVisible{ true };
		};

	public:
		// QML model roles
		enum class TableHeaderModelRoles : int {
			// defined for consistency in implementation of data() function
			DisplayData = Qt::DisplayRole,
			// type of sort on the column, check SortTypes enum
			SortType = Qt::UserRole,
			// true if filter is applied
			IsFiltered = Qt::UserRole + 1,
			// value of the applied filter
			FilterValue = Qt::UserRole + 2,
			// column width
			ColumnWidth = Qt::UserRole + 3,
			// column visibility
			IsVisible = Qt::UserRole + 4
		};

		// ctor
		explicit TableHeaderModel(SQLTableViewImpl* tableView, QObject* parent = nullptr);

		// QAbstractItemModel interface start

		// basic functionality
		QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
		QModelIndex parent(const QModelIndex& child) const override;
		int rowCount(const QModelIndex& parent = QModelIndex()) const override;
		int columnCount(const QModelIndex& parent = QModelIndex()) const override;

		// get data
		QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

		// set data
		bool setData(const QModelIndex& index, const QVariant& value, int role) override;

		// QML roles
		QHash<int, QByteArray> roleNames() const override;

		// QAbstractItemModel interface end

		// set data function for use under QML
		Q_INVOKABLE bool setData(int row, int column, const QVariant& value, const QString& role);

		// used to sort given column
		Q_INVOKABLE bool sortColumn(int column);

		// used to apply filter to a given column
		Q_INVOKABLE bool setColumnFilter(int column, const QString& filterValue);

		// used to set width for a given column
		Q_INVOKABLE bool setColumnWidth(int column, qreal columnWidth);

		// returns width for given column
		Q_INVOKABLE qreal columnWidth(int column) const;

		// returns name for given column
		Q_INVOKABLE QString columnName(int column) const;

		// changes column visiblity
		Q_INVOKABLE bool setColumnVisibility(int column, bool visibility);

		// returns visibility of given column
		Q_INVOKABLE bool columnVisibility(int column) const;

		// check is given column index is valid
		bool columnIsValid(int column) const;

		// returns iterators for m_columnProperties vector
		auto begin()
		{
			return m_columnProperties.begin();
		};
		auto end()
		{
			return m_columnProperties.end();
		};

		// returns const iterators for m_columnProperties vector
		const auto cbegin() const
		{
			return m_columnProperties.cbegin();
		};
		const auto cend() const
		{
			return m_columnProperties.cend();
		};

		// m_modelWrapper getter
		Q_INVOKABLE ColumnsSettingsWrapper* modelWrapper() const;

		// sets width for given column, doeas not emit dataChanged or columnWidthChanged signals
		void initializeColumnWidth(int column, qreal columnWidth);

		// WARNING: absolutely no boundary checks
		ColumnProperties& at(int column)
		{
			return m_columnProperties[column];
		}

		// m_excludedColumns getter / setter
		QSet<int> excludedColumns() const;
		void setExcludedColumns(const QSet<int>& excludedColumns);

	private:
		// internal handler to the table view
		SQLTableViewImpl* m_tableView;

		// container of column properties
		std::vector<ColumnProperties> m_columnProperties;

		// cached rolenames
		mutable QHash<int, QByteArray> m_cachedRoleNames;
		mutable QHash<QString, int> m_reversedRoleNames;

		// wrapper for column settings
		ColumnsSettingsWrapper* m_modelWrapper;

		// set of globally hidden columns
		QSet<int> m_excludedColumns;

	signals:
		// emited when column sort was applied
		void columnSorted(int column, SortType sortType);

		// emited when column filter was changed
		void filterChanged(int column, QString filterValue);

		// emited when column width was changed
		void columnWidthChanged(int column, qreal columnWidth);

		// emited when column visibility was changed
		void columnVisibilityChanged(int column, bool isVisible);

		// emited when columns were initialized
		void initialized();

		// emited when excluded columns were set
		void excludedColumnsSet();
	};

}   // namespace IzSQLTableView

#endif   // IZSQLTABLEVIEW_TABLEHEADERMODEL_H
