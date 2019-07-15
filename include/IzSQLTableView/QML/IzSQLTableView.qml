import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQml.Models 2.12
import QtQuick.Layouts 1.12
import IzSQLTableView 1.0
import IzLibrary 1.0

import "../JS/ObjectCreator.js" as ObjectCreator

// TODO: implementacja przywracania domyślnych ustawień tabelki
// TODO: porozbijać na komponenty
// TODO: rozwiąć problem z ustawianiem kolorów tabeli w zależności od theme aplikacji
// TODO: implementacja delegata w CPP
// TODO: roszczerzenie funckjonalności ustawień kolumn o wszystko co jest w HeaderTableModel'u
SQLTableViewImpl {
	id: root

	// view handler
	readonly property alias view: tableViewInternal

	// if true refresh button will be clicakble
	property bool refreshButtonEnabled: true

	// used to export data
	function exportData(filePath, preserveColumnWidths, addAutofilters, selectedRowsOnly) {
		root.exportDataToXLSX(filePath, preserveColumnWidths, addAutofilters, selectedRowsOnly);
	}

	// returns base cell color
	function cellColor(highlighted, isSelected, row, column) {
		if (highlighted) {
			return Qt.tint(root.baseColor, Qt.rgba(root.tintColor.r, root.tintColor.g, root.tintColor.b, 0.256));
		} else {
			if (isSelected) {
				return Qt.tint(root.baseColor, Qt.rgba(root.tintColor.r, root.tintColor.g, root.tintColor.b, 0.512));
			} else {
				if (root.alternatingRowColors) {
					if (row % 2 !== 0) {
						return Qt.lighter(root.baseColor, root.lighterFactor);
					} else {
						return Qt.darker(root.baseColor, root.darkerFactor);
					}
				} else {
					return root.baseColor;
				}
			}
		}
	}

	// emited when cell was clicked
	signal clicked(int row, int column);

	// emited when cell was double clicked
	signal doubleClicked(int row, int column);

	tintColor: "#079300"
	baseColor: root.Material.theme === Material.Dark ? "#7f8a93" : "#f7f7f7"
	darkerFactor: root.Material.theme === Material.Dark ? 1.7 : 1.0
	lighterFactor: 0.9
	focus: true

	onRelayout: {
		tableViewInternal.forceLayout();
		tableHeader.forceLayout();
	}

	onFilterFieldsVisibleChanged: {
		tableHeader.forceLayout();
	}

	onDataExportEnded: {
		if (result) {
			IzToast.show(root, qsTr("Data exported"), "ok");
		} else {
			IzToast.show(root, qsTr("Data export error"), "error");
		}
	}

	// TODO: to rozwiązać zdecydowanie inaczej
	Material.onThemeChanged: {
		quickSettings.baseColor = root.Material.theme === Material.Dark ? "#7f8a93" : "#f7f7f7";
		quickSettings.darkerFactor = root.Material.theme === Material.Dark ? 1.7 : 1.0;
	}

	states: [
		State {
			name: "refreshingData"
			when: tableViewInternal.model.source.isRefreshingData

			PropertyChanges {
				target: tableViewInternal
				opacity: 0
			}

			PropertyChanges {
				target: tableHeader
				opacity: 0
			}

			PropertyChanges {
				target: dataRefreshIndicator
				visible: true
			}

			PropertyChanges {
				target: dataRefreshRectangle
				state: "visible"
			}
		},

		State {
			name: "filteringData"
			when: tableViewInternal.model.isFiltering

			PropertyChanges {
				target: dataFilteringIndicator
				visible: true
			}
		},

		State {
			name: "exportingData"
			when: root.isExportingData

			PropertyChanges {
				target: dataRefreshIndicator
				visible: true
			}
		}
	]

	ProgressBar {
		id: dataRefreshIndicator

		anchors {
			bottom: parent.bottom
			bottomMargin: tableFooter.height
			left: parent.left
			right: parent.right
		}

		visible: false
		z: tableViewInternal.z + 10
		indeterminate: root.isExportingData && !root.isSavingData ? false : true
		from: 0
		to: root.dataSizeToExport
		value: root.exportedDataSetSize
	}

	ProgressBar {
		id: dataFilteringIndicator

		anchors {
			top: parent.top
			topMargin: tableHeaderContainer.height
			left: parent.left
			right: parent.right
		}

		indeterminate: true
		visible: false
		z: tableViewInternal.z + 10
	}

	Rectangle {
		id: emptyModelIndicator

		width: 200
		height: 40
		anchors.centerIn: parent
		visible: root.model.source.count === 0 && !root.model.source.isRefreshingData

		color: root.Material.background
		border.width: 1
		border.color: Material.color(Material.Blue)

		IzText {
			anchors.centerIn: parent
			text: qsTr("No results")
		}
	}

	Rectangle {
		id: dataRefreshRectangle

		anchors {
			top: parent.top
			topMargin: tableHeaderContainer.height + mainLayout.spacing
			bottom: parent.bottom
			bottomMargin: tableFooter.height + dataRefreshIndicator.height
			left: parent.left
			right: parent.right
		}

		z: mainLayout.z + 1000
		visible: true
		color: Material.color(Material.Grey)
		opacity: 0.0

		states: [
			State {
				name: "visible"

				PropertyChanges {
					target: dataRefreshRectangle
					opacity: 0.20
				}
			}
		]

		transitions: Transition {
			PropertyAnimation {
				property: "opacity"
				duration: 500
			}
		}
	}

	QuickSettings {
		id: quickSettings

		z: tableViewInternal.z + 1000
		anchors {
			right: parent.right
			bottom: parent.bottom
			bottomMargin: tableFooter.height + mainLayout.spacing
		}

		onDarkerFactorChanged: { root.darkerFactor = darkerFactor; }
		onLighterFactorChanged: { root.lighterFactor = lighterFactor; }
		onBaseColorChanged: { root.baseColor = baseColor; }
		onTintColorChanged: { root.tintColor = tintColor; }
		onAlternatingRowColorsChanged: { root.alternatingRowColors = alternatingRowColors; }
		onFiltersVisibleChanged: { root.filterFieldsVisible = filtersVisible; }
		onSelectionModeChanged: { root.selectionMode = selectionMode; }

		Component.onCompleted: {
			quickSettings.darkerFactor = root.darkerFactor;
			quickSettings.lighterFactor = root.lighterFactor;
			quickSettings.baseColor = root.baseColor;
			quickSettings.tintColor = root.tintColor;
			quickSettings.alternatingRowColors = root.alternatingRowColors;
			quickSettings.filtersVisible = root.filterFieldsVisible;
			quickSettings.selectionMode = root.selectionMode;
		}

		onResetSettings: {
			quickSettings.tintColor = "#079300";
			quickSettings.baseColor = root.Material.theme === Material.Dark ? "#7f8a93" : "#f7f7f7";
			quickSettings.darkerFactor = root.Material.theme === Material.Dark ? 1.7 : 1.0;
			quickSettings.lighterFactor = 0.9;
			quickSettings.alternatingRowColors = true;
			quickSettings.filtersVisible = true;
		}
	}

	ColumnsSettings {
		id: columnsSettings

		z: tableViewInternal.z + 1000
		height: 30
		width: 30
		anchors {
			right: parent.right
			rightMargin: 5
			top: parent.top
			topMargin: tableHeaderContainer.height + 5
			bottomMargin: tableFooter.height + mainLayout.spacing
		}

		onShowColumnSettings: {
			var columnSettings = ObjectCreator.create(root, "qrc:/include/IzSQLTableView/QML/ColumnsSettingsPopup.qml",
														{
															"anchors.centerIn": root,
															"headerModel": tableHeader.model,
															"globalFilterValue": Qt.binding(function(){
																return root.globalColumnFilter;
															})
														});
			if (columnSettings) {
				columnSettings.changeColumnWidth.connect(tableHeader.model.setColumnWidth);
				columnSettings.setGlobalColumnFilter.connect(root.setGlobalColumnFilterDefinition);
				columnSettings.open();
			}
		}
	}

	ColumnLayout {
		id: mainLayout

		anchors.fill: parent

		IzRectangle {
			id: tableHeaderContainer

			signal columnSizeSet(int column, int columnSize)

			Layout.preferredHeight: root.filterFieldsVisible ? 55 : 30
			Layout.fillWidth: true

			TableView {
				id: tableHeader

				anchors.fill: parent

				model: root.header
				enabled: root.state === ""
				clip: true
				interactive: false
				contentWidth: root.tableContentWidth
				boundsMovement: Flickable.StopAtBounds
				rowSpacing: 1
				contentX: tableViewInternal.contentX

				columnWidthProvider: function (column) {
					return root.columnWidth(column);
				}

				rowHeightProvider: function (row) {
					return root.filterFieldsVisible ? 55 : 30;
				}

				delegate: HeaderDelegate {
					tableView: root

					filterFieldVisible: root.filterFieldsVisible

					onSetColumnWidth: {
						tableHeader.model.setColumnWidth(column, width);
					}

					onFilterColumn: {
						tableHeader.model.setColumnFilter(column, filter);
					}

					onSortColumn: {
						tableHeader.model.sortColumn(column);
					}
				}
			}
		}

		Item {
			id: dataRefreshPadding

			Layout.fillWidth: true
			Layout.fillHeight: true

			visible: !tableViewInternal.visible
		}

		TableView {
			id: tableViewInternal

			// prerefresh contentY
			property real prerefreshContentY: 0
			property bool wasRefreshingData: false

			Layout.fillWidth: true
			Layout.fillHeight: true

			enabled: root.state === ""
			focus: true
			clip: true
			interactive: !quickSettings.hovered
			contentWidth: tableHeader.contentWidth
			boundsMovement: tableHeader.boundsMovement
			rowSpacing: tableHeader.rowSpacing

			columnWidthProvider: function (column) {
				return root.columnWidth(column);
			}

			rowHeightProvider: function (row) {
				return root.rowHeight;
			}

			ScrollBar.vertical: ScrollBar {
				minimumSize: 0.1
				width: 5
			}

			ScrollBar.horizontal: ScrollBar {
				minimumSize: 0.1
				height: 5
			}

			Keys.onPressed: {
				if (event.key === Qt.Key_A && (event.modifiers & Qt.ControlModifier)) {
					root.selectAll();
				} else if (event.key === Qt.Key_Escape) {
					root.clearSelection();
				} else if (event.key === Qt.Key_Up) {
					root.currentIndexUp();
				} else if (event.key === Qt.Key_Down) {
					root.currentIndexDown();
				} else if (event.key === Qt.Key_Left) {
					root.currentIndexLeft();
				} else if (event.key === Qt.Key_Right) {
					root.currentIndexRight();
				} else if (event.key === Qt.Key_C && (event.modifiers & Qt.ControlModifier)) {
					root.copyDataToClipboard();
				}
			}

			delegate: Rectangle {

				implicitHeight: root.Material.delegateHeight

				property bool highlighted: root.indexIsHighlighted(row, column, root.highlightedRow, root.highlightedColumn)

				MouseArea {
					id: mouseArea

					anchors.fill: parent

					hoverEnabled: true
					propagateComposedEvents: true

					onContainsMouseChanged: {
						if (containsMouse) {
							root.highlightedRow = row;
							root.highlightedColumn = column;
						}
					}

					onReleased: {
						tableViewInternal.forceActiveFocus();
						if (mouse.modifiers & Qt.ShiftModifier) {
							root.select(root.selectionFromIndexes(root.currentIndex, root.createIndex(row, column)), ItemSelectionModel.Select);
						} else if (mouse.modifiers & Qt.ControlModifier) {
							root.select(root.createIndex(row, column), ItemSelectionModel.Select | ItemSelectionModel.Toggle);
						} else {
							root.select(root.createIndex(row, column), ItemSelectionModel.ClearAndSelect | ItemSelectionModel.Current);
						}
					}

					onClicked: {
						root.clicked(row, column);
					}

					onDoubleClicked: {
						root.doubleClicked(row, column);
					}

					Loader {
						anchors {
							fill: parent
							leftMargin: 2
							rightMargin: 2
						}

						source: root.cellDelegateProvider(row, column, displayData)
					}
				}

				color: Qt.tint(root.cellColor(highlighted, isSelected, row, column), root.cellColorProvider(row, column, displayData))
			}

			model: root.model

			Connections {
				target: root.model.source

				onAboutToRefreshData: {
					tableViewInternal.prerefreshContentY = tableViewInternal.contentY;
				}

				onDataRefreshStarted: {
					// WARNING: hack
					tableViewInternal.wasRefreshingData = true;
					tableViewInternal.contentX = 0;
					tableViewInternal.contentY = 0;
				}

				onDataRefreshEnded: {
					// restore contentY after refresh
					if (result && tableViewInternal.contentHeight > tableViewInternal.prerefreshContentY) {
						tableViewInternal.contentY = tableViewInternal.prerefreshContentY;
					}
				}
			}

			Connections {
				target: root.model

				onIsFilteringChanged: {
					// WARNING: hack na dziewczne zachowanie QML'owej tabelki
					if(!root.model.isFiltering && !tableViewInternal.wasRefreshingData) {
						tableViewInternal.contentY = 0;
					} else if (!root.model.isFiltering && tableViewInternal.wasRefreshingData) {
						tableViewInternal.wasRefreshingData = false;
					}
				}
			}
		}

		IzRectangle {
			id: tableFooter

			Layout.preferredHeight: 30
			Layout.fillWidth: true

			enabled: root.state === ""

			RowLayout {
				anchors {
					fill: parent
					leftMargin: 4
					rightMargin: 4
				}

				IzText {
					Layout.fillHeight: true
					Layout.preferredWidth: 120

					text: qsTr("Loaded: ") + root.loadedRows
				}

				IzText {
					Layout.fillHeight: true
					Layout.preferredWidth: 120

					text: qsTr("Selected: ") + root.selectionCount
				}

				IzText {
					Layout.fillHeight: true
					Layout.preferredWidth: 120

					text: qsTr("Row: ") + root.currentRow
				}

				IzText {
					Layout.fillHeight: true
					Layout.preferredWidth: 120

					text: qsTr("Column: ") + root.currentColumn
				}

				IzText {
					Layout.fillHeight: true
					Layout.preferredWidth: 300

					text: qsTr("State: <b>") + root.stateDescription + "</b>"
				}

				Item {
					Layout.fillHeight: true
					Layout.fillWidth: true
				}

				IzButton {
					Layout.preferredHeight: 26
					Layout.preferredWidth: 26

					fontIcon: "\uf450"
					tooltip: qsTr("Refresh data")
					enabled: root.model.source.queryIsValid && root.refreshButtonEnabled

					onReleased: {
						root.model.source.refreshData();
					}
				}

				IzButton {
					Layout.preferredHeight: 26
					Layout.preferredWidth: 26
					Layout.alignment: Qt.AlignRight

					enabled: root.dataExportEnabled && root.model.source.count !== 0
					fontIcon: "\uf21b"
					tooltip: qsTr("Export data to Excel'a")

					onReleased: {
						var exportDialog = ObjectCreator.create(root, "qrc:/include/IzSQLTableView/QML/ExcelExportPopup.qml",
																{
																	"anchors.centerIn": root
																});
						if (exportDialog) {
							exportDialog.open();
						}
						exportDialog.onExportData.connect(root.exportData);
					}
				}

				IzButton {
					Layout.preferredHeight: 26
					Layout.preferredWidth: 26

					fontIcon: tableViewInternal.activeFocus ? "\uf30c" : "\uf310"
					tooltip: tableViewInternal.activeFocus ? qsTr("Keyboard navigation enabled") : qsTr("Keyboard navigation disabled")

					onReleased: {
						tableViewInternal.forceActiveFocus();
					}
				}
			}
		}
	}
}
