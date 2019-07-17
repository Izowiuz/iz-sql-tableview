#include "SQLTableViewImpl.h"

#include <QClipboard>
#include <QDebug>
#include <QGuiApplication>
#include <QItemSelectionModel>
#include <QJSValue>
#include <QQmlEngine>
#include <QRegularExpression>
#include <QtConcurrent>

#include "IzSQLUtilities/SQLTableModel.h"
#include "IzSQLUtilities/SQLTableProxyModel.h"

#include "TableHeaderModel.h"
#include "TableHeaderProxyModel.h"

#ifdef LIBXLSXWRITER_ENABLED
#include "xlsxwriter.h"
#endif

IzSQLTableView::SQLTableViewImpl::SQLTableViewImpl(QQuickItem* parent)
	: QQuickItem(parent)
	, m_model(new IzSQLUtilities::SQLTableProxyModel(this))
	, m_header(new TableHeaderProxyModel(this, this))
	, m_selectionModel(new QItemSelectionModel(m_model, this))
{
	// proxy setup
	m_model->setSelectionModel(m_selectionModel);

	// reflow connects
	connect(this, &QQuickItem::widthChanged, this, [this]() {
		if (width() > m_tableContentWidth) {
			m_tableReflowTimer.start();
		} else if (width() <= m_tableContentWidth) {
			m_tableReflowTimer.start();
		}
	});

	// reflow timer setup
	m_tableReflowTimer.setInterval(250);
	m_tableReflowTimer.setSingleShot(true);

	// reflow timer setup
	connect(&m_tableReflowTimer, &QTimer::timeout, this, [this]() {
		calculateLastColumnWidth();
		calculateTableContentWidth();
		emit relayout();
	});

	// model connects
	connect(m_model, &IzSQLUtilities::SQLTableProxyModel::isFilteringChanged, this, [this]() {
		if (m_model->isFiltering()) {
			setViewState(QStringLiteral("filtering data"));
		} else {
			setDefaultViewState();
		}
	});

	// model connects
	connect(m_model->source(), &IzSQLUtilities::SQLTableModel::sqlQueryStarted, this, [this]() {
		setViewState(QStringLiteral("executing query"));
	});

	connect(m_model->source(), &IzSQLUtilities::SQLTableModel::sqlQueryReturned, this, [this]() {
		setViewState(QStringLiteral("loading data"));
	});

	connect(m_model->source(), &IzSQLUtilities::SQLTableModel::aboutToRefreshData, this, [this]() {
		emit aboutToRefreshData();
	});

	connect(m_model->source(), &IzSQLUtilities::SQLTableModel::dataRefreshStarted, this, [this]() {
		m_selectionModel->clearSelection();
		m_selectionModel->clearCurrentIndex();
		emit refreshStarted();
	});

	connect(m_model->source(), &IzSQLUtilities::SQLTableModel::dataRefreshEnded, this, [this](bool result) {
		if (result) {
			setDefaultViewState();
		} else {
			setViewState(QStringLiteral("query error :["));
		}
		emit refreshEnded(result);
	});

	connect(m_model->source(), &IzSQLUtilities::SQLTableModel::rowsLoaded, this, [this](int rowsCount) {
		m_loadedRows = rowsCount;
		emit loadedRowsChanged();
	});

	connect(m_model->source(), &IzSQLUtilities::SQLTableModel::databaseTypeChanged, this, &SQLTableViewImpl::databaseTypeChanged);

	connect(m_model->source(), &IzSQLUtilities::SQLTableModel::connectionParametersChanged, this, &SQLTableViewImpl::connectionParametersChanged);

	// header model connects
	connect(m_header->source(), &TableHeaderModel::columnSorted, this, [this](int column, SortType sortType) {
		switch (sortType) {
		case SortType::Ascending:
			// as 'column' ia a source column we have to transform it to the proxy column
			m_model->sort(m_header->proxyColumn(column), Qt::AscendingOrder);
			emit currentRowChanged();
			emit currentColumnChanged();
			break;
		case SortType::Descending:
			// as 'column' ia a source column we have to transform it to the proxy column
			m_model->sort(m_header->proxyColumn(column), Qt::DescendingOrder);
			emit currentRowChanged();
			emit currentColumnChanged();
			break;
		default:
			break;
		}
	});

	connect(m_header->source(), &TableHeaderModel::columnVisibilityChanged, this, [this](int column, QString columnName, bool columnVisibility) {
		// TODO: tu można byłoby zastanowić się czy koniecznie trzeba usuwać zaznaczenie i currentIndex
		if (m_selectionModel->hasSelection()) {
			m_selectionModel->clearSelection();
			m_selectionModel->clearCurrentIndex();
		}
		if (!columnVisibility) {
			if (!m_hiddenColumns.contains(columnName)) {
				m_hiddenColumns.push_back(columnName);
			}
		} else {
			m_hiddenColumns.removeAll(columnName);
		}
		calculateNaturalTableContentWidth();
		calculateTableContentWidth();
	});

	connect(m_header->source(), &TableHeaderModel::filterChanged, this, [this](int column, const QString& filterValue) {
		if (m_header->source()->at(column).isFiltered) {
			m_model->addColumnFilter(column, QRegularExpression(filterValue, QRegularExpression::PatternOption::CaseInsensitiveOption));
		} else {
			m_model->removeColumnFilter(column);
		}
	});

	// table dimensions calculation, &TableHeaderModel::initialized is emited post setup of header
	connect(m_header->source(), &TableHeaderModel::initialized, this, [this]() {
		QMapIterator<QString, QVariant> it(m_cachedColumnWidths);
		while (it.hasNext()) {
			it.next();
			// TODO: tu można byłoby sprawdzać czy ten QVariant to faktycznie qreal
			if (m_model->source()->indexFromColumnName(it.key()) != -1) {
				m_header->source()->initializeColumnWidth(m_model->source()->indexFromColumnName(it.key()), it.value().toReal());
			}
		}

		// TODO: przydałoby się to ogarnąc w jakiś bardziej racjonalny sposób
		initializeGlobalColumnFilter();

		for (const auto& column : qAsConst(m_hiddenColumns)) {
			for (int i{ 0 }; i < m_header->source()->columnCount(); ++i) {
				if (m_header->source()->data(m_header->source()->index(0, i), static_cast<int>(TableHeaderModel::TableHeaderModelRoles::DisplayData)).toString() == column) {
					m_header->source()->setColumnVisibility(i, false);
					continue;
				}
			}
		}

		calculateNaturalTableContentWidth();
		calculateTableContentWidth();
	});

	connect(m_header->source(), &TableHeaderModel::columnWidthChanged, this, [this](int column, qreal width) {
		cacheColumnWidth(column, width);
		emit columnWidthsChanged();
		calculateNaturalTableContentWidth();
		calculateTableContentWidth();
		emit relayout();
	});

	// selection model connects
	connect(m_selectionModel, &QItemSelectionModel::currentChanged, this, [this]() {
		emit currentIndexChanged();
	});

	connect(m_selectionModel, &QItemSelectionModel::currentRowChanged, this, [this]() {
		emit currentRowChanged();
	});

	connect(m_selectionModel, &QItemSelectionModel::currentColumnChanged, this, [this]() {
		emit currentColumnChanged();
	});

	// TODO: upewnić się, czy aby na pewno nie da się zrobić tego lepiej :[
	connect(m_selectionModel, &QItemSelectionModel::selectionChanged, this, [this](auto selected, auto deselected) {
		// set current index depending on the selected indexes
		if (m_selectionModel->hasSelection()) {
			if (selected.isEmpty()) {
				m_selectionModel->setCurrentIndex(m_selectionModel->selection().first().topLeft(), QItemSelectionModel::Select);
			} else {
				m_selectionModel->setCurrentIndex(selected.first().topLeft(), QItemSelectionModel::Select);
			}
		} else {
			m_selectionModel->clearCurrentIndex();
		}

		selected.merge(deselected, QItemSelectionModel::Select);
		for (const auto& range : qAsConst(selected)) {
			m_model->dataChanged(range.topLeft(), range.bottomRight(), { static_cast<int>(IzSQLUtilities::SQLTableModel::SQLTableModelRoles::IsSelected) });
		}
		emit selectionChanged();
	});
}

bool IzSQLTableView::SQLTableViewImpl::indexIsHighlighted(int row, int column, int highlightedRow, int highlightedColumn) const
{
	Q_UNUSED(highlightedRow)
	Q_UNUSED(highlightedColumn)
	switch (m_selectionMode) {
	case IzSQLTableView::TableSelectionMode::SelectRows:
		return m_highlightedRow == row;
	case IzSQLTableView::TableSelectionMode::SelectCells:
		return m_highlightedColumn == column && m_highlightedRow == row;
	case IzSQLTableView::TableSelectionMode::SelectColumns:
		return m_highlightedColumn == column;
	default:
		return false;
	}
}

IzSQLTableView::TableHeaderProxyModel* IzSQLTableView::SQLTableViewImpl::header() const
{
	return m_header;
}

IzSQLUtilities::SQLTableProxyModel* IzSQLTableView::SQLTableViewImpl::model() const
{
	return m_model;
}

IzSQLTableView::TableSelectionMode IzSQLTableView::SQLTableViewImpl::selectionMode() const
{
	return m_selectionMode;
}

void IzSQLTableView::SQLTableViewImpl::setSelectionMode(TableSelectionMode selectionMode)
{
	if (m_selectionMode != selectionMode) {
		m_selectionMode = selectionMode;
		emit selectionModeChanged();
	}
}

int IzSQLTableView::SQLTableViewImpl::highlightedColumn() const
{
	return m_highlightedColumn;
}

void IzSQLTableView::SQLTableViewImpl::setHighlightedColumn(int highlightedColumn)
{
	if (m_highlightedColumn != highlightedColumn) {
		m_highlightedColumn = highlightedColumn;
		emit highlightedColumnChanged();
	}
}

QVariantMap IzSQLTableView::SQLTableViewImpl::columnWidths() const
{
	return m_cachedColumnWidths;
}

void IzSQLTableView::SQLTableViewImpl::setColumnWidths(const QVariantMap& columnWidths)
{
	if (m_cachedColumnWidths != columnWidths) {
		m_cachedColumnWidths = columnWidths;
	}
}

qreal IzSQLTableView::SQLTableViewImpl::columnWidth(int column) const
{
	if (column == m_header->columnCount() - 1) {
		return lastColumnWidth();
	}

	// because we get 'raw' column numbers from QML we have to transform them trought the header proxy
	return m_header->source()->columnWidth(m_header->sourceColumn(column));
}

QVariant IzSQLTableView::SQLTableViewImpl::cellData(int row, int column) const
{
	// WARNING: hmm ...podejrzane, że tu nie przechodzi to przez source()
	return m_model->data(m_model->index(row, column));
}

QVariant IzSQLTableView::SQLTableViewImpl::cellData(int row, const QString& columnName) const
{
	return m_model->source()->data(m_model->source()->index(m_model->sourceRow(row), m_model->source()->indexFromColumnName(columnName)));
}

QVariantList IzSQLTableView::SQLTableViewImpl::selectedCellsData(const QString& columnName) const
{
	if (!m_selectionModel->hasSelection()) {
		return {};
	}

	const auto rows = m_selectionModel->selectedRows();
	QVariantList ret;
	ret.reserve(rows.size());

	for (const auto& row : qAsConst(rows)) {
		ret.push_back(cellData(row.row(), columnName));
	}

	return ret;
}

qreal IzSQLTableView::SQLTableViewImpl::defaultColumnWidth() const
{
	return m_defaultColumnWidth;
}

void IzSQLTableView::SQLTableViewImpl::setDefaultColumnWidth(qreal defaultColumnWidth)
{
	if (m_defaultColumnWidth != defaultColumnWidth) {
		m_defaultColumnWidth = defaultColumnWidth;
		emit defaultColumnWidthChanged();
	}
}

QColor IzSQLTableView::SQLTableViewImpl::baseColor() const
{
	return m_baseColor;
}

void IzSQLTableView::SQLTableViewImpl::setBaseColor(const QColor& baseColor)
{
	if (m_baseColor != baseColor) {
		m_baseColor = baseColor;
		emit baseColorChanged();
	}
}

qreal IzSQLTableView::SQLTableViewImpl::tableContentWidth() const
{
	return m_tableContentWidth;
}

QModelIndex IzSQLTableView::SQLTableViewImpl::currentIndex() const
{
	return m_selectionModel->currentIndex();
}

int IzSQLTableView::SQLTableViewImpl::currentRow() const
{
	return m_selectionModel->currentIndex().row();
}

int IzSQLTableView::SQLTableViewImpl::currentColumn() const
{
	return m_selectionModel->currentIndex().column();
}

bool IzSQLTableView::SQLTableViewImpl::hasSelection() const
{
	return m_selectionModel->hasSelection();
}

bool IzSQLTableView::SQLTableViewImpl::isColumnSelected(int column) const
{
	return m_selectionModel->isColumnSelected(column, {});
}

bool IzSQLTableView::SQLTableViewImpl::isRowSelected(int row) const
{
	return m_selectionModel->isColumnSelected(row, {});
}

bool IzSQLTableView::SQLTableViewImpl::isSelected(int row, int column) const
{
	return m_selectionModel->isSelected(m_model->index(row, column));
}

QItemSelection IzSQLTableView::SQLTableViewImpl::selection() const
{
	return m_selectionModel->selection();
}

void IzSQLTableView::SQLTableViewImpl::select(const QModelIndex& index, int flags)
{
	switch (m_selectionMode) {
	case TableSelectionMode::SelectCells:
		m_selectionModel->select(index, static_cast<QItemSelectionModel::SelectionFlags>(flags));
		break;
	case TableSelectionMode::SelectRows:
		m_selectionModel->select(index, static_cast<QItemSelectionModel::SelectionFlags>(flags | QItemSelectionModel::Rows));
		break;
	case TableSelectionMode::SelectColumns:
		m_selectionModel->select(index, static_cast<QItemSelectionModel::SelectionFlags>(flags | QItemSelectionModel::Columns));
		break;
	}
}

void IzSQLTableView::SQLTableViewImpl::select(const QItemSelection& selection, int flags)
{
	switch (m_selectionMode) {
	case TableSelectionMode::SelectCells:
		m_selectionModel->select(selection, static_cast<QItemSelectionModel::SelectionFlags>(flags));
		break;
	case TableSelectionMode::SelectRows:
		m_selectionModel->select(selection, static_cast<QItemSelectionModel::SelectionFlags>(flags | QItemSelectionModel::Rows));
		break;
	case TableSelectionMode::SelectColumns:
		m_selectionModel->select(selection, static_cast<QItemSelectionModel::SelectionFlags>(flags | QItemSelectionModel::Columns));
		break;
	}
}

QItemSelection IzSQLTableView::SQLTableViewImpl::selectionFromIndexes(int topLeftRow, int topLeftColumn, int bottomRightRow, int bottomRightColumn) const
{
	return QItemSelection(m_model->index(topLeftRow, topLeftColumn), m_model->index(bottomRightRow, bottomRightColumn));
}

QItemSelection IzSQLTableView::SQLTableViewImpl::selectionFromIndexes(const QModelIndex& topLeft, const QModelIndex& bottomRight) const
{
	return QItemSelection(topLeft, bottomRight);
}

QModelIndex IzSQLTableView::SQLTableViewImpl::createIndex(int row, int column) const
{
	return m_model->index(row, column);
}

int IzSQLTableView::SQLTableViewImpl::selectionCount() const
{
	switch (m_selectionMode) {
	case TableSelectionMode::SelectCells:
		return m_selectionModel->selectedIndexes().size();
	case TableSelectionMode::SelectRows:
		return m_selectionModel->selectedRows().size();
	case TableSelectionMode::SelectColumns:
		return m_selectionModel->selectedColumns().size();
	}
	return 0;
}

void IzSQLTableView::SQLTableViewImpl::testScript(const QString& script)
{
	if (qmlEngine(this) == nullptr) {
		qCritical() << "qmlEngine is invalid.";
		return;
	}

	qDebug() << script;
	qDebug() << qmlEngine(this)->evaluate(script).toVariant();
}

bool IzSQLTableView::SQLTableViewImpl::alternatingRowColors() const
{
	return m_alternatingRowColors;
}

void IzSQLTableView::SQLTableViewImpl::setAlternatingRowColors(bool alternatingRowColors)
{
	if (m_alternatingRowColors != alternatingRowColors) {
		m_alternatingRowColors = alternatingRowColors;
		emit alternatingRowColorsChanged();
	}
}

qreal IzSQLTableView::SQLTableViewImpl::lighterFactor() const
{
	return m_lighterFactor;
}

void IzSQLTableView::SQLTableViewImpl::setLighterFactor(qreal lighterFactor)
{
	if (m_lighterFactor != lighterFactor) {
		m_lighterFactor = lighterFactor;
		emit lighterFactorChanged();
	}
}

qreal IzSQLTableView::SQLTableViewImpl::darkerFactor() const
{
	return m_darkerFactor;
}

void IzSQLTableView::SQLTableViewImpl::setDarkerFactor(qreal darkerFactor)
{
	if (m_darkerFactor != darkerFactor) {
		m_darkerFactor = darkerFactor;
		emit darkerFactorChanged();
	}
}

QColor IzSQLTableView::SQLTableViewImpl::tintColor() const
{
	return m_tintColor;
}

void IzSQLTableView::SQLTableViewImpl::setTintColor(const QColor& tintColor)
{
	if (m_tintColor != tintColor) {
		m_tintColor = tintColor;
		emit tintColorChanged();
	}
}

void IzSQLTableView::SQLTableViewImpl::calculateTableContentWidth()
{
	qreal res{ 0 };

	for (int i{ 0 }; i < m_header->columnCount(); ++i) {
		res += columnWidth(i);
	}

	if (res != m_tableContentWidth) {
		m_tableContentWidth = res;
		emit tableContentWidthChanged();
	}
}

void IzSQLTableView::SQLTableViewImpl::calculateNaturalTableContentWidth()
{
	qreal res{ 0 };
	for (int i{ 0 }; i < m_header->columnCount(); ++i) {
		res += m_header->index(0, i).data(static_cast<int>(TableHeaderModel::TableHeaderModelRoles::ColumnWidth)).toReal();
	}
	m_naturalTableContentWidth = res;
	calculateLastColumnWidth();
}

void IzSQLTableView::SQLTableViewImpl::calculateLastColumnWidth()
{
	if (m_header->columnCount() == 0) {
		return;
	}

	qreal lastColumnWidth = m_header->index(0, m_header->columnCount() - 1).data(static_cast<int>(TableHeaderModel::TableHeaderModelRoles::ColumnWidth)).toReal();

	// clang-format off
	if (width() - m_naturalTableContentWidth > 0) {
		m_lastColumnWidth = lastColumnWidth + (width() - m_naturalTableContentWidth);
	} else {
		m_lastColumnWidth = lastColumnWidth;
	}
	// clang-format on
}

qreal IzSQLTableView::SQLTableViewImpl::lastColumnWidth() const
{
	return m_lastColumnWidth;
}

QStringList IzSQLTableView::SQLTableViewImpl::hiddenColumns() const
{
	return m_hiddenColumns;
}

void IzSQLTableView::SQLTableViewImpl::setHiddenColumns(const QStringList& hiddenColumns)
{
	if (m_hiddenColumns != hiddenColumns) {
		m_hiddenColumns = hiddenColumns;
		for (const auto& column : qAsConst(m_hiddenColumns)) {
			for (int i{ 0 }; i < m_header->source()->columnCount(); ++i) {
				if (m_header->source()->data(m_header->source()->index(0, i), static_cast<int>(TableHeaderModel::TableHeaderModelRoles::DisplayData)).toString() == column) {
					m_header->source()->setColumnVisibility(i, false);
					continue;
				}
			}
		}
		emit hiddenColumnsChanged();
	}
}

QVariantMap IzSQLTableView::SQLTableViewImpl::connectionParameters() const
{
	return m_model->source()->connectionParameters();
}

void IzSQLTableView::SQLTableViewImpl::setConnectionParameters(const QVariantMap &connectionParameters)
{
	return m_model->source()->setConnectionParameters(connectionParameters);
}

QString IzSQLTableView::SQLTableViewImpl::databaseType() const
{
	return m_databaseType;
}

void IzSQLTableView::SQLTableViewImpl::setDatabaseType(const QString &databaseType)
{
	if (databaseType == QStringLiteral("MSSQL")) {
		m_databaseType = databaseType;
		m_model->source()->setDatabaseType(IzSQLUtilities::DatabaseType::MSSQL);
	} else if (databaseType == QStringLiteral("PSQL")) {
		m_databaseType = databaseType;
		m_model->source()->setDatabaseType(IzSQLUtilities::DatabaseType::PSQL);
	} else if (databaseType == QStringLiteral("SQLITE")) {
		m_databaseType = databaseType;
		m_model->source()->setDatabaseType(IzSQLUtilities::DatabaseType::SQLITE);
	} else {
		qWarning() << "Got invalid database type:" << databaseType;
	}
}

QJSValue IzSQLTableView::SQLTableViewImpl::cellDelegateProvider() const
{
	return m_cellDelegateProvider;
}

void IzSQLTableView::SQLTableViewImpl::setCellDelegateProvider(const QJSValue& callback)
{
	if (!callback.isCallable()) {
		qWarning() << "cellDelegateProvider must be a callable function.";
		return;
	}
	m_cellDelegateProvider = callback;
	emit cellDelegateProviderChanged();
}

QString IzSQLTableView::SQLTableViewImpl::globalColumnFilterDefinition() const
{
	return m_globalColumnFilterDefinition;
}

void IzSQLTableView::SQLTableViewImpl::setGlobalColumnFilterDefinition(const QString& globalColumnFilterDefinition)
{
	if (m_globalColumnFilterDefinition != globalColumnFilterDefinition) {
		m_globalColumnFilterDefinition = globalColumnFilterDefinition;
		emit globalColumnFilterDefinitionChanged();
		initializeGlobalColumnFilter();
		calculateNaturalTableContentWidth();
		calculateTableContentWidth();
	}
}

QJSValue IzSQLTableView::SQLTableViewImpl::cellColorProvider() const
{
	return m_cellColorProvider;
}

void IzSQLTableView::SQLTableViewImpl::setCellColorProvider(const QJSValue& callback)
{
	if (!callback.isCallable()) {
		qWarning() << "cellColorProvider must be a callable function.";
		return;
	}
	m_cellColorProvider = callback;
	emit cellColorProviderChanged();
}

void IzSQLTableView::SQLTableViewImpl::setDefaultViewState()
{
	m_stateDescription = QStringLiteral("idle");
	emit stateChanged();
}

void IzSQLTableView::SQLTableViewImpl::setViewState(const QString& stateDescription)
{
	m_stateDescription = stateDescription;
	emit stateChanged();
}

void IzSQLTableView::SQLTableViewImpl::cacheColumnWidth(int column, qreal width)
{
	m_cachedColumnWidths.insert(m_model->source()->columnNameFromIndex(column), width);
}

void IzSQLTableView::SQLTableViewImpl::initializeGlobalColumnFilter()
{
	if (m_header->source()->columnCount() == 0) {
		return;
	}

	if (m_globalColumnFilterDefinition.isEmpty()) {
		m_header->source()->setExcludedColumns({});
		m_model->setExcludedColumns({});
	} else {
		QRegularExpression regExp(m_globalColumnFilterDefinition);
		if (regExp.isValid()) {
			QSet<int> excludedColumns;

			// TODO: po dodaniu funkcjonalności zamiany kolejności kolumn ten fragment kodu trzeba będzie zmienić
			for (int i = 0; i < m_header->source()->columnCount(); ++i) {
				if (m_header->source()->columnName(i).contains(regExp)) {
					excludedColumns.insert(i);
				}
			}

			m_header->source()->setExcludedColumns(excludedColumns);
		}
	}
}

QVariantMap IzSQLTableView::SQLTableViewImpl::columnNames() const
{
	return m_model->source()->columnNameColumnAliasMap();
}

void IzSQLTableView::SQLTableViewImpl::setColumnNames(const QVariantMap& columnNames)
{
	m_model->source()->setColumnNameColumnAliasMap(columnNames);
	emit columnNamesChanged();
}

bool IzSQLTableView::SQLTableViewImpl::dataExportEnabled() const
{
	return m_dataExportEnabled;
}

void IzSQLTableView::SQLTableViewImpl::setDataExportEnabled(bool dataExportEnabled)
{
	if (m_dataExportEnabled != dataExportEnabled) {
		m_dataExportEnabled = dataExportEnabled;
		emit dataExportEnabledChanged();
	}
}

bool IzSQLTableView::SQLTableViewImpl::filterFieldsVisible() const
{
	return m_filterFieldsVisible;
}

void IzSQLTableView::SQLTableViewImpl::setFilterFieldsVisible(bool filterFieldsVisible)
{
	if (m_filterFieldsVisible != filterFieldsVisible) {
		m_filterFieldsVisible = filterFieldsVisible;
		emit filterFieldsVisibleChanged();
	}
}

int IzSQLTableView::SQLTableViewImpl::rowHeight() const
{
	return m_rowHeight;
}

void IzSQLTableView::SQLTableViewImpl::setRowHeight(int rowHeight)
{
	if (m_rowHeight != rowHeight) {
		m_rowHeight = rowHeight;
		emit rowHeightChanged();
	}
}

int IzSQLTableView::SQLTableViewImpl::loadedRows() const
{
	return m_loadedRows;
}

void IzSQLTableView::SQLTableViewImpl::exportDataToXLSX(const QUrl& filePath, bool preserveColumnWidths, bool addAutoFilters, bool selectedRowsOnly)
{
#ifdef LIBXLSXWRITER_ENABLED
	if (m_dataExportFuture.isRunning()) {
		qCritical() << "Last data export operation is still running. Aborting.";
		emit dataExportEnded(false);
		return;
	}

	if (selectedRowsOnly) {
		m_dataSizeToExport = m_selectionModel->selectedRows().size();
	} else {
		m_dataSizeToExport = m_model->rowCount();
	}
	emit dataSizeToExportChanged();

	m_exportedDataSetSize = 0;
	emit exportedDataSetSizeChanged();
	setViewState(QStringLiteral("przetwarzanie danych"));

	m_dataExportFuture = QtConcurrent::run([this,
											filePath             = filePath,
											preserveColumnWidths = preserveColumnWidths,
											addAutoFilters       = addAutoFilters,
											selectedRowsOnly     = selectedRowsOnly] {
		bool result{ true };

		m_isExportingData = true;
		emit isExportingDataChanged();

		// workbook
		lxw_workbook* workbook = workbook_new(filePath.toLocalFile().toStdString().c_str());

		// worksheet
		lxw_worksheet* worksheet = workbook_add_worksheet(workbook, "Dane");

		// date formatting
		lxw_format* dateFormat = workbook_add_format(workbook);
		format_set_num_format(dateFormat, "mm-dd-yyyy hh:mm:ss");

		// column index | column name | column width
		std::vector<std::tuple<int, QString, qreal>> headerDefinition;
		headerDefinition.reserve(m_header->columnCount());

		for (int i{ 0 }; i < m_header->columnCount(); ++i) {
			headerDefinition.emplace_back(std::make_tuple(i,
														  m_header->data(m_header->index(0, i), static_cast<int>(TableHeaderModel::TableHeaderModelRoles::DisplayData)).toString(),
														  m_header->data(m_header->index(0, i), static_cast<int>(TableHeaderModel::TableHeaderModelRoles::ColumnWidth)).toReal()));
		}

		for (const auto& column : headerDefinition) {
			if (preserveColumnWidths) {
				// 8 being magic scaling number between pixels in QML and size in EXCEL
				worksheet_set_column(worksheet, std::get<0>(column), std::get<0>(column), std::get<2>(column) / 8, nullptr);
			} else {
				worksheet_set_column(worksheet, 0, 1, 30, nullptr);
			}
			worksheet_write_string(worksheet, 0, std::get<0>(column), std::get<1>(column).toStdString().c_str(), nullptr);
		}

		int exportedDataSize{ 0 };
		for (int i{ 0 }; i < m_model->rowCount(); ++i) {
			int currentColumn = 0;

			if (selectedRowsOnly && !m_model->data(m_model->index(i, 0), static_cast<int>(IzSQLUtilities::SQLTableModel::SQLTableModelRoles::IsSelected)).toBool()) {
				continue;
			}

			for (const auto& column : headerDefinition) {
				auto modelIndex = m_model->index(i, std::get<0>(column));
				switch (static_cast<QMetaType::Type>(m_model->data(modelIndex).type())) {
				case QMetaType::QDateTime: {
					QDateTime tdt = m_model->data(modelIndex).toDateTime();
					if (tdt.isValid()) {
						lxw_datetime dt = {
							tdt.date().year(),
							tdt.date().month(),
							tdt.date().day(),
							tdt.time().hour(),
							tdt.time().minute(),
							static_cast<double>(tdt.time().second())
						};
						worksheet_write_datetime(worksheet, exportedDataSize + 1, currentColumn, &dt, dateFormat);
					}
					break;
				}
				case QMetaType::Double: {
					worksheet_write_number(worksheet, exportedDataSize + 1, currentColumn, m_model->data(modelIndex).toDouble(), nullptr);
					break;
				}
				case QMetaType::Float: {
					worksheet_write_number(worksheet, exportedDataSize + 1, currentColumn, m_model->data(modelIndex).toDouble(), nullptr);
					break;
				}
				case QMetaType::Int: {
					worksheet_write_number(worksheet, exportedDataSize + 1, currentColumn, m_model->data(modelIndex).toInt(), nullptr);
					break;
				}
				case QMetaType::UInt: {
					worksheet_write_number(worksheet, exportedDataSize + 1, currentColumn, m_model->data(modelIndex).toUInt(), nullptr);
					break;
				}
				default:
					worksheet_write_string(worksheet, exportedDataSize + 1, currentColumn, m_model->data(modelIndex).toString().toStdString().c_str(), nullptr);
				}
				currentColumn++;
			}
			exportedDataSize++;
			if (exportedDataSize % 10 == 0) {
				m_exportedDataSetSize = exportedDataSize;
				emit exportedDataSetSizeChanged();
			}
		}

		// autofilters
		if (addAutoFilters) {
			// NOTICE: why -1?
			worksheet_autofilter(worksheet, 0, 0, exportedDataSize, m_header->columnCount() - 1);
		}

		m_isSavingData = true;
		emit isSavingDataChanged();
		setViewState(QStringLiteral("zapisywanie danych"));

		// close workbook
		if (workbook_close(workbook) != LXW_NO_ERROR) {
			result = false;
		}

		m_isSavingData = false;
		emit isSavingDataChanged();
		setDefaultViewState();

		m_isExportingData = false;
		emit isExportingDataChanged();
		emit dataExportEnded(result);
		m_exportedDataSetSize = 0;
		emit exportedDataSetSizeChanged();
	});

#else
	Q_UNUSED(filePath)
	Q_UNUSED(preserveColumnWidths)
	Q_UNUSED(addAutoFilters)
	Q_UNUSED(selectedRowsOnly)
	qCritical() << "libxlsxwriter is not enabled but data export was requested.";
	emit dataExportEnded(false);
#endif
}

bool IzSQLTableView::SQLTableViewImpl::multiselection() const
{
	return m_multiselection;
}

void IzSQLTableView::SQLTableViewImpl::setMultiselection(bool multiselection)
{
	if (m_multiselection != multiselection) {
		m_multiselection = multiselection;
		emit multiselectionChanged();
	}
}

void IzSQLTableView::SQLTableViewImpl::selectAll()
{
	m_selectionModel->select({ m_model->index(0, 0), m_model->index(m_model->rowCount() - 1, m_model->columnCount() - 1) }, QItemSelectionModel::Select);
}

void IzSQLTableView::SQLTableViewImpl::clearSelection()
{
	m_selectionModel->clearSelection();
}

void IzSQLTableView::SQLTableViewImpl::currentIndexUp()
{
	switch (m_selectionMode) {
	case TableSelectionMode::SelectCells:
		// index selection moves freely
		if (!m_selectionModel->currentIndex().isValid()) {
			m_selectionModel->select(m_model->index(m_model->rowCount() - 1, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Current);
		} else {
			m_selectionModel->select(m_model->index(m_selectionModel->currentIndex().row() - 1,
													m_selectionModel->currentIndex().column()),
									 QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Current);
		}
		break;
	case TableSelectionMode::SelectRows:
		// row selection moves freely
		if (!m_selectionModel->currentIndex().isValid()) {
			m_selectionModel->select(m_model->index(m_model->rowCount() - 1, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Current | QItemSelectionModel::Rows);
		} else {
			m_selectionModel->select(m_model->index(m_selectionModel->currentIndex().row() - 1,
													m_selectionModel->currentIndex().column()),
									 QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Current | QItemSelectionModel::Rows);
		}
		break;
	case TableSelectionMode::SelectColumns:
		// nothing happens
		break;
	}
}

void IzSQLTableView::SQLTableViewImpl::currentIndexDown()
{
	switch (m_selectionMode) {
	case TableSelectionMode::SelectCells:
		// index selection moves freely
		if (!m_selectionModel->currentIndex().isValid()) {
			m_selectionModel->select(m_model->index(0, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Current);
		} else {
			m_selectionModel->select(m_model->index(m_selectionModel->currentIndex().row() + 1,
													m_selectionModel->currentIndex().column()),
									 QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Current);
		}
		break;
	case TableSelectionMode::SelectRows:
		// row selection moves freely
		if (!m_selectionModel->currentIndex().isValid()) {
			m_selectionModel->select(m_model->index(0, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Current | QItemSelectionModel::Rows);
		} else {
			m_selectionModel->select(m_model->index(m_selectionModel->currentIndex().row() + 1,
													m_selectionModel->currentIndex().column()),
									 QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Current | QItemSelectionModel::Rows);
		}
		break;
	case TableSelectionMode::SelectColumns:
		// nothing happens
		break;
	}
}

void IzSQLTableView::SQLTableViewImpl::currentIndexLeft()
{
	switch (m_selectionMode) {
	case TableSelectionMode::SelectCells:
		// index selection moves freely
		if (!m_selectionModel->currentIndex().isValid()) {
			m_selectionModel->select(m_model->index(0, m_model->columnCount() - 1), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Current);
		} else {
			m_selectionModel->select(m_model->index(m_selectionModel->currentIndex().row(),
													m_selectionModel->currentIndex().column() - 1),
									 QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Current);
		}
		break;
	case TableSelectionMode::SelectRows:
		// nothing happens
		break;
	case TableSelectionMode::SelectColumns:
		// column selection moves freely
		if (!m_selectionModel->currentIndex().isValid()) {
			m_selectionModel->select(m_model->index(0, m_model->columnCount() - 1), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Current | QItemSelectionModel::Columns);
		} else {
			m_selectionModel->select(m_model->index(m_selectionModel->currentIndex().row(),
													m_selectionModel->currentIndex().column() - 1),
									 QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Current | QItemSelectionModel::Columns);
		}
		break;
	}
}

void IzSQLTableView::SQLTableViewImpl::currentIndexRight()
{
	switch (m_selectionMode) {
	case TableSelectionMode::SelectCells:
		// index selection moves freely
		if (!m_selectionModel->currentIndex().isValid()) {
			m_selectionModel->select(m_model->index(0, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Current);
		} else {
			m_selectionModel->select(m_model->index(m_selectionModel->currentIndex().row(),
													m_selectionModel->currentIndex().column() + 1),
									 QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Current);
		}
		break;
	case TableSelectionMode::SelectRows:
		// nothing happens
		break;
	case TableSelectionMode::SelectColumns:
		// column selection moves freely
		if (!m_selectionModel->currentIndex().isValid()) {
			m_selectionModel->select(m_model->index(0, 0), QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Current | QItemSelectionModel::Columns);
		} else {
			m_selectionModel->select(m_model->index(m_selectionModel->currentIndex().row(),
													m_selectionModel->currentIndex().column() + 1),
									 QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Current | QItemSelectionModel::Columns);
		}
		break;
	}
}

void IzSQLTableView::SQLTableViewImpl::copyDataToClipboard() const
{
	if (!m_selectionModel->hasSelection()) {
		return;
	}

	if (m_selectionMode == TableSelectionMode::SelectCells) {
		QGuiApplication::clipboard()->setText(m_model->data(m_model->index(m_selectionModel->currentIndex().row(), m_selectionModel->currentIndex().column())).toString());
	} else if (m_selectionMode == TableSelectionMode::SelectRows) {
		QString tmp;

		for (int i{ 0 }; i < m_header->source()->columnCount(); ++i) {
			if (m_header->source()->data(m_header->source()->index(0, i), static_cast<int>(TableHeaderModel::TableHeaderModelRoles::IsVisible)).toBool()) {
				tmp += cellData(m_selectionModel->currentIndex().row(), m_header->proxyColumn(i)).toString() + QStringLiteral("\t");
			}
		}

		QGuiApplication::clipboard()->setText(tmp);
	} else if (m_selectionMode == TableSelectionMode::SelectColumns) {
		QString tmp;

		for (int i{ 0 }; i < m_model->rowCount(); ++i) {
			tmp += cellData(i, m_selectionModel->currentIndex().column()).toString() + QStringLiteral("\r\n");
		}

		QGuiApplication::clipboard()->setText(tmp);
	}
}

int IzSQLTableView::SQLTableViewImpl::highlightedRow() const
{
	return m_highlightedRow;
}

void IzSQLTableView::SQLTableViewImpl::setHighlightedRow(int highlightedRow)
{
	if (m_highlightedRow != highlightedRow) {
		m_highlightedRow = highlightedRow;
		emit highlightedRowChanged();
	}
}

void IzSQLTableView::SQLTableViewImpl::classBegin()
{
	auto engine = qmlEngine(this);

	if (engine != nullptr) {
		m_cellColorProvider    = engine->evaluate(QStringLiteral("( function(row, column, value) { return \"transparent\"; } )"));
		m_cellDelegateProvider = engine->evaluate(QStringLiteral("( function(row, column, value) { return \"qrc:/include/IzSQLTableView/QML/DefaultDelegate.qml\"; } )"));
	} else {
		qWarning() << "Got invalid qmlEngine hadler. Providers will be undefined.";
	}
}
