#ifndef IZSQLTABLEVIEW_SQLTABLEVIEWIMPL_H
#define IZSQLTABLEVIEW_SQLTABLEVIEWIMPL_H

#include <atomic>

#include <QFuture>
#include <QQuickItem>
#include <QTimer>

#include "IzSQLTableView/IzSQLTableViewPlugin_Enums.h"

class QItemSelectionModel;
namespace IzSQLUtilities
{
	class SQLTableProxyModel;
}

namespace IzSQLTableView
{
	class TableHeaderProxyModel;
	class UserColumnWidthsWrapper;

	class SQLTableViewImpl : public QQuickItem
	{
		friend class UserColumnWidthsWrapper;

		Q_OBJECT
		Q_DISABLE_COPY(SQLTableViewImpl)

		// current selection mode of the table
		Q_PROPERTY(IzSQLTableView::TableSelectionMode selectionMode READ selectionMode WRITE setSelectionMode NOTIFY selectionModeChanged FINAL)

		// currently highlighted row - used to properly highlight data in table
		Q_PROPERTY(int highlightedRow READ highlightedRow WRITE setHighlightedRow NOTIFY highlightedRowChanged FINAL)

		// currently highlighted column - used to properly highlight data in table
		Q_PROPERTY(int highlightedColumn READ highlightedColumn WRITE setHighlightedColumn NOTIFY highlightedColumnChanged FINAL)

		// model of this view
		Q_PROPERTY(IzSQLUtilities::SQLTableProxyModel* model READ model CONSTANT)

		// header of this view
		Q_PROPERTY(TableHeaderProxyModel* header READ header CONSTANT)

		// default column width
		Q_PROPERTY(qreal defaultColumnWidth READ defaultColumnWidth WRITE setDefaultColumnWidth NOTIFY defaultColumnWidthChanged FINAL)

		// base color of the table
		Q_PROPERTY(QColor baseColor READ baseColor WRITE setBaseColor NOTIFY baseColorChanged FINAL)

		// total content width
		Q_PROPERTY(qreal tableContentWidth READ tableContentWidth NOTIFY tableContentWidthChanged FINAL)

		// current index of the table
		Q_PROPERTY(QModelIndex currentIndex READ currentIndex NOTIFY currentIndexChanged FINAL)

		// current row of the table
		Q_PROPERTY(int currentRow READ currentRow NOTIFY currentRowChanged FINAL)

		// current column of the table
		Q_PROPERTY(int currentColumn READ currentColumn NOTIFY currentColumnChanged FINAL)

		// table selection
		Q_PROPERTY(QItemSelection selection READ selection NOTIFY selectionChanged FINAL)

		// table selection count
		Q_PROPERTY(int selectionCount READ selectionCount NOTIFY selectionChanged FINAL)

		// table selection true / false
		Q_PROPERTY(bool hasSelection READ hasSelection NOTIFY selectionChanged FINAL)

		// used to tint base color for the highlight, alpha is ignored
		Q_PROPERTY(QColor tintColor READ tintColor WRITE setTintColor NOTIFY tintColorChanged FINAL)

		// used with alternating row colors to darken the base color
		Q_PROPERTY(qreal darkerFactor READ darkerFactor WRITE setDarkerFactor NOTIFY darkerFactorChanged FINAL)

		// used with alternating row colors to lighten the base color
		Q_PROPERTY(qreal lighterFactor READ lighterFactor WRITE setLighterFactor NOTIFY lighterFactorChanged FINAL)

		// if true table view will draw alternating colors on delegates
		Q_PROPERTY(bool alternatingRowColors READ alternatingRowColors WRITE setAlternatingRowColors NOTIFY alternatingRowColorsChanged FINAL)

		// column widths
		Q_PROPERTY(QVariantMap columnWidths READ columnWidths WRITE setColumnWidths NOTIFY columnWidthsChanged FINAL)

		// column names
		Q_PROPERTY(QVariantMap columnNames READ columnNames WRITE setColumnNames NOTIFY columnNamesChanged FINAL)

		// if true table will allow for multpile selection of indexes
		Q_PROPERTY(bool multiselection READ multiselection WRITE setMultiselection NOTIFY multiselectionChanged FINAL)

		// number of currently loaded rows in model
		Q_PROPERTY(int loadedRows READ loadedRows NOTIFY loadedRowsChanged FINAL)

		// true if data is curently being exported
		Q_PROPERTY(bool isExportingData MEMBER m_isExportingData NOTIFY isExportingDataChanged FINAL)

		// amount of currently exported data
		Q_PROPERTY(int dataSizeToExport MEMBER m_dataSizeToExport NOTIFY dataSizeToExportChanged FINAL)

		// amount of elements to be exported to external file
		Q_PROPERTY(int exportedDataSetSize MEMBER m_exportedDataSetSize NOTIFY exportedDataSetSizeChanged FINAL)

		// height of the table row
		Q_PROPERTY(int rowHeight READ rowHeight WRITE setRowHeight NOTIFY rowHeightChanged FINAL)

		// if ture filter fields are visible
		Q_PROPERTY(bool filterFieldsVisible READ filterFieldsVisible WRITE setFilterFieldsVisible NOTIFY filterFieldsVisibleChanged FINAL)

		// if true data can be exported to XLSX file
		Q_PROPERTY(bool dataExportEnabled READ dataExportEnabled WRITE setDataExportEnabled NOTIFY dataExportEnabledChanged FINAL)

		// regexp definition used to globally hid table columns independently of column state
		Q_PROPERTY(QString globalColumnFilter READ globalColumnFilterDefinition WRITE setGlobalColumnFilterDefinition NOTIFY globalColumnFilterDefinitionChanged FINAL)

		// true if view is currently loading data
		Q_PROPERTY(bool isSavingData MEMBER m_isSavingData NOTIFY isSavingDataChanged FINAL)

		// informational state of the view
		Q_PROPERTY(QString stateDescription MEMBER m_stateDescription NOTIFY stateChanged)

		// cell color provider
		Q_PROPERTY(QJSValue cellColorProvider READ cellColorProvider WRITE setCellColorProvider NOTIFY cellColorProviderChanged FINAL)

		// cell delegate provider
		Q_PROPERTY(QJSValue cellDelegateProvider READ cellDelegateProvider WRITE setCellDelegateProvider NOTIFY cellDelegateProviderChanged FINAL)

		// default delegate for cell
		Q_PROPERTY(QUrl defaultDelegateURL MEMBER m_defaultDelegateURL CONSTANT FINAL)

		// list of hidden columns
		Q_PROPERTY(QStringList hiddenColumns READ hiddenColumns WRITE setHiddenColumns NOTIFY hiddenColumnsChanged FINAL)

		// current type of database connection
		// pass through to AbstarctSQLModel
		Q_PROPERTY(QString databaseType READ databaseType WRITE setDatabaseType NOTIFY databaseTypeChanged FINAL)

		// current connection parameters - empty parameters = parameter are read from dynamic properties of qApp
		// pass through to AbstarctSQLModel
		Q_PROPERTY(QVariantMap connectionParameters READ connectionParameters WRITE setConnectionParameters NOTIFY connectionParametersChanged FINAL)

	public:
		// ctor
		explicit SQLTableViewImpl(QQuickItem* parent = nullptr);

		// TODO: to raczej powinno się inaczej zrobić
		// returns true if given delegate is supposed to be highlighted
		// highlightedRow and highlightedColumn are only here to force QML bindings to this function
		Q_INVOKABLE bool indexIsHighlighted(int row, int column, int highlightedRow, int highlightedColumn) const;

		// returns model for this view
		IzSQLUtilities::SQLTableProxyModel* model() const;

		// returns header for this view
		TableHeaderProxyModel* header() const;

		// m_selectionMode getter / setter
		IzSQLTableView::TableSelectionMode selectionMode() const;
		void setSelectionMode(IzSQLTableView::TableSelectionMode selectionMode);

		// m_highlightedRow getter / setter
		int highlightedRow() const;
		void setHighlightedRow(int highlightedRow);

		// m_highlightedColumn getter / setter
		int highlightedColumn() const;
		void setHighlightedColumn(int highlightedColumn);

		// returns widths for this table
		Q_INVOKABLE QVariantMap columnWidths() const;

		// sets column widths for this table
		Q_INVOKABLE void setColumnWidths(const QVariantMap& columnWidths);

		// returns width for given column index
		Q_INVOKABLE qreal columnWidth(int column) const;

		// returns data for given row and column
		Q_INVOKABLE QVariant cellData(int row, int column) const;

		// returns data for given row and column name
		Q_INVOKABLE QVariant cellData(int row, const QString& columnName) const;

		// returns selected cells data for given column name
		Q_INVOKABLE QVariantList selectedCellsData(const QString& columnName) const;

		// m_defaultColumnWidth getter / setter
		qreal defaultColumnWidth() const;
		void setDefaultColumnWidth(qreal defaultColumnWidth);

		// m_baseColor setter / getter
		QColor baseColor() const;
		void setBaseColor(const QColor& baseColor);

		// content width of this table
		qreal tableContentWidth() const;

		// QItemSelection warpper functions start

		// returns current index
		QModelIndex currentIndex() const;

		// returns current row
		int currentRow() const;

		// returns current column
		int currentColumn() const;

		// returns true if table view has selection
		bool hasSelection() const;

		// returns true if given column is selected
		Q_INVOKABLE bool isColumnSelected(int column) const;

		// returns true if given row is selected
		Q_INVOKABLE bool isRowSelected(int row) const;

		// returns true if given cell is selected
		Q_INVOKABLE bool isSelected(int row, int column) const;

		// selection of the table
		Q_INVOKABLE QItemSelection selection() const;

		// used to select indexes on the table
		Q_INVOKABLE void select(const QModelIndex& index, int flags);
		Q_INVOKABLE void select(const QItemSelection& selection, int flags);

		// QItemSelection warpper functions end

		// constructs QItemSelection from top-left and bottom-right indexes
		Q_INVOKABLE QItemSelection selectionFromIndexes(int topLeftRow, int topLeftColumn, int bottomRightRow, int bottomRightColumn) const;
		Q_INVOKABLE QItemSelection selectionFromIndexes(const QModelIndex& topLeft, const QModelIndex& bottomRight) const;

		// constructs QModelIndex from row and column
		Q_INVOKABLE QModelIndex createIndex(int row, int column) const;

		// returns selection count for table
		int selectionCount() const;

		// m_tintColor getter / setter
		QColor tintColor() const;
		void setTintColor(const QColor& tintColor);

		// m_darkerFactor getter / setter
		qreal darkerFactor() const;
		void setDarkerFactor(qreal darkerFactor);

		// m_lighterFactor getter / setter
		qreal lighterFactor() const;
		void setLighterFactor(qreal lighterFactor);

		// m_alternatingRowColors getter / setter
		bool alternatingRowColors() const;
		void setAlternatingRowColors(bool alternatingRowColors);

		// dev / debug functions
		Q_INVOKABLE void testScript(const QString& script);

		// m_multiselection getter / setter
		bool multiselection() const;
		void setMultiselection(bool multiselection);

		// helper function: selects all data
		Q_INVOKABLE void selectAll();

		// helper function: clears selection
		Q_INVOKABLE void clearSelection();

		// helper function: moves current index -1 position (upward), selects last index if nothing is selected
		Q_INVOKABLE void currentIndexUp();

		// helper function: moves current index +1 position (downward), selects first index if nothing is selected
		Q_INVOKABLE void currentIndexDown();

		// helper function: moves current index -1 position (left), selects last index if nothing is selected
		Q_INVOKABLE void currentIndexLeft();

		// helper function: moves current index +1 position (right), selects first index if nothing is selected
		Q_INVOKABLE void currentIndexRight();

		// copies current data to system clipboard
		Q_INVOKABLE void copyDataToClipboard() const;

		// m_loadedRows getter
		int loadedRows() const;

		// exports data to XLSX format
		Q_INVOKABLE void exportDataToXLSX(const QUrl& filePath, bool preserveColumnWidths, bool addAutoFilters, bool selectedRowsOnly);

		// m_rowHeight getter / setter
		int rowHeight() const;
		void setRowHeight(int rowHeight);

		// m_filterFieldsVisible getter / setter
		bool filterFieldsVisible() const;
		void setFilterFieldsVisible(bool filterFieldsVisible);

		// getter / setter
		bool dataExportEnabled() const;
		void setDataExportEnabled(bool dataExportEnabled);

		// model's column names getter / setter
		QVariantMap columnNames() const;
		void setColumnNames(const QVariantMap& columnNames);

		// m_globalColumnFilterDefinition
		QString globalColumnFilterDefinition() const;
		Q_INVOKABLE void setGlobalColumnFilterDefinition(const QString& globalColumnFilterDefinition);

		// cellColorProvider getter / setter
		QJSValue cellColorProvider() const;
		void setCellColorProvider(const QJSValue& callback);

		// cellDelegateProvider getter / setter
		QJSValue cellDelegateProvider() const;
		void setCellDelegateProvider(const QJSValue& callback);

		// m_hiddenColumns getter / setter
		QStringList hiddenColumns() const;
		void setHiddenColumns(const QStringList& hiddenColumns);

		// connectionParameters setter / getter
		QVariantMap connectionParameters() const;
		void setConnectionParameters(const QVariantMap& connectionParameters);

		// m_databaseType setter / getter
		QString databaseType() const;
		void setDatabaseType(const QString& databaseType);

		// QQuickItem interface start

		void classBegin() override;

		// QQuickItem interface end

	private:
		// sets default view state
		void setDefaultViewState();

		// sets state of the view
		void setViewState(const QString& stateDescription);

		// used to cache column width for given index
		void cacheColumnWidth(int column, qreal width);

		// initializes global column filter
		void initializeGlobalColumnFilter();

		// current selection mode of the table
		IzSQLTableView::TableSelectionMode m_selectionMode{ TableSelectionMode::SelectRows };

		// currently highlighted row - used to properly highlight data in table
		int m_highlightedRow{ -1 };

		// currently highlighted column - used to properly highlight data in table
		int m_highlightedColumn{ -1 };

		// internal model of this view
		IzSQLUtilities::SQLTableProxyModel* m_model;

		// internal header ot this view
		TableHeaderProxyModel* m_header;

		// default width for columns
		qreal m_defaultColumnWidth{ 120 };

		// base table color
		QColor m_baseColor;

		// used to tint base color for the highlight, alpha is ignored
		QColor m_tintColor;

		// used with alternating row colors to darken the base color
		qreal m_darkerFactor{ 1.7 };

		// used with alternating row colors to lighten the base color
		qreal m_lighterFactor{ 0.9 };

		// if true table view will draw alternating colors on delegates
		bool m_alternatingRowColors{ true };

		// calculates table content width
		void calculateTableContentWidth();

		// calculates m_naturalTableContentWidth
		void calculateNaturalTableContentWidth();

		// calculates last column width - depends on the QQuickItem width() function and m_naturalTableContentWidth
		void calculateLastColumnWidth();

		// m_lastColumnWidth getter
		qreal lastColumnWidth() const;

		// table content width
		mutable qreal m_tableContentWidth{ 0 };

		// non modified width of all table columns
		qreal m_naturalTableContentWidth{ 0 };

		// width of the last column
		qreal m_lastColumnWidth{ 0 };

		// table reflow timer
		QTimer m_tableReflowTimer;

		// internal selection model
		QItemSelectionModel* m_selectionModel;

		// if true table will allow for multile selection of indexes
		bool m_multiselection{ false };

		// cached column widths
		QVariantMap m_cachedColumnWidths;

		// number of currently loaded rows in model
		int m_loadedRows{ 0 };

		// true if data is curently being exported
		std::atomic<bool> m_isExportingData{ false };

		// data export future
		QFuture<void> m_dataExportFuture;

		// height of the table row
		int m_rowHeight{ 30 };

		// if ture filter fields are visible
		bool m_filterFieldsVisible{ true };

		// if true data can be exported to XLSX file
		bool m_dataExportEnabled{ true };

		// regexp definition used to globally hid table columns independently of column state
		QString m_globalColumnFilterDefinition;

		// data size to export during export operation
		std::atomic<int> m_dataSizeToExport{ 0 };

		// currently exported data size
		std::atomic<int> m_exportedDataSetSize{ 0 };

		// true if model is currently saving exported data
		std::atomic<bool> m_isSavingData{ false };

		// informational state of the view
		QString m_stateDescription{ QStringLiteral("idle") };

		// cell color provider
		QJSValue m_cellColorProvider;

		// cell delegate provider
		QJSValue m_cellDelegateProvider;

		// default delegate URL
		QUrl m_defaultDelegateURL{ QStringLiteral("qrc:/include/IzSQLTableView/QML/DefaultDelegate.qml") };

		// hidden columns
		QStringList m_hiddenColumns;

		// current databaseType
		QString m_databaseType;

	signals:
		// Q_PROPERTY changed signals
		void selectionModeChanged();
		void highlightedRowChanged();
		void highlightedColumnChanged();
		void defaultColumnWidthChanged();
		void baseColorChanged();
		void tableContentWidthChanged();
		void currentIndexChanged();
		void currentRowChanged();
		void currentColumnChanged();
		void selectionChanged();
		void tintColorChanged();
		void darkerFactorChanged();
		void lighterFactorChanged();
		void alternatingRowColorsChanged();
		void columnWidthsChanged();
		void multiselectionChanged();
		void loadedRowsChanged();
		void isExportingDataChanged();
		void rowHeightChanged();
		void filterFieldsVisibleChanged();
		void dataExportEnabledChanged();
		void columnNamesChanged();
		void globalColumnFilterDefinitionChanged();
		void dataSizeToExportChanged();
		void exportedDataSetSizeChanged();
		void isSavingDataChanged();
		void stateChanged();
		void cellColorProviderChanged();
		void cellDelegateProviderChanged();
		void hiddenColumnsChanged();
		void databaseTypeChanged();
		void connectionParametersChanged();

		// emited when table layout has to be recalculated
		void relayout();

		// emited when data export ended
		void dataExportEnded(bool result);

		// wrapped signals from source model state changes
		void aboutToRefreshData();
		void refreshStarted();
		void refreshEnded(bool result);
	};
}   // namespace IzSQLTableView

#endif   // IZSQLTABLEVIEW_SQLTABLEVIEWIMPL_H
