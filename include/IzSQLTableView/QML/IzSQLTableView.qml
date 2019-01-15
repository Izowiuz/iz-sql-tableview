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

	// used to export data
	function exportData(filePath, preserveColumnWidths, addAutofilters, selectedRowsOnly) {
		root.exportDataToXLSX(filePath, preserveColumnWidths, addAutofilters, selectedRowsOnly);
	}

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
			IzToast.show(root, qsTr("Dane wyeksportowano poprawnie"), "ok");
		} else {
			IzToast.show(root, qsTr("Nastąpił błąd przy eksportowaniu danych"), "error");
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
			when: tableViewInternal.model.source.isRefreshing

			PropertyChanges {
				target: tableFooter
				enabled: false
			}

			PropertyChanges {
				target: tableViewInternal
				enabled: false
				visible: false
			}

			PropertyChanges {
				target: tableHeaderContainer
				enabled: false
			}

			PropertyChanges {
				target: tableHeader
				opacity: 0
			}

			PropertyChanges {
				target: quickSettings
				enabled: false
			}

			PropertyChanges {
				target: columnsSettings
				enabled: false
			}

			PropertyChanges {
				target: dataRefreshIndicator
				visible: true
			}

			PropertyChanges {
				target: dataRefreshPadding
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

			PropertyChanges {
				target: tableFooter
				enabled: false
			}
		},

		State {
			name: "exportingData"
			when: root.isExportingData

			PropertyChanges {
				target: tableFooter
				enabled: false
			}

			PropertyChanges {
				target: tableViewInternal
				enabled: false
			}

			PropertyChanges {
				target: tableHeaderContainer
				enabled: false
			}

			PropertyChanges {
				target: quickSettings
				enabled: false
			}

			PropertyChanges {
				target: columnsSettings
				enabled: false
			}

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

		indeterminate: true
		visible: false
		z: tableViewInternal.z + 10
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
		visible: root.model.source.count === 0 && !root.model.source.isRefreshing

		color: root.Material.background
		border.width: 1
		border.color: Material.color(Material.Blue)

		IzText {
			anchors.centerIn: parent
			text: qsTr("Brak pozycji do wyświetlenia")
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
					opacity: 0.35
				}
			}
		]

		transitions: Transition {
			PropertyAnimation { property: "opacity" }
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
				columnSettings.changeColumnWidth.connect(root.setColumnWidth);
				columnSettings.setGlobalColumnFilter.connect(root.setGlobalColumnFilterDefinition);
				columnSettings.open();
			}
		}
	}

	ColumnLayout {
		id: mainLayout

		anchors.fill: parent

		IzBoxContainer {
			id: tableHeaderContainer

			signal columnSizeSet(int column, int columnSize)

			Layout.preferredHeight: root.filterFieldsVisible ? 55 : 30
			Layout.fillWidth: true

			TableView {
				id: tableHeader

				anchors.fill: parent

				model: root.header

				clip: true
				interactive: false
				contentWidth: tableViewInternal.contentWidth
				boundsMovement: tableViewInternal.boundsMovement
				rowSpacing: tableViewInternal.rowSpacing
				contentX: tableViewInternal.contentX
				columnWidthProvider: function (column) {
					// TODO: wtf?
					if (column >= tableHeader.model.columnCount()) { return 1; }
					return root.columnWidth(column);
				}
				rowHeightProvider: function (row) {
					return root.filterFieldsVisible ? 55 : 30;
				}

				delegate: HeaderDelegate {
					tableView: root

					filterFieldVisible: root.filterFieldsVisible

					onSetColumnWidth: {
						root.setColumnWidth(column, width);
					}

					onFilterColumn: {
						root.setColumnFilter(column, filter);
					}

					onSortColumn: {
						root.sortColumn(column);
					}
				}

				Behavior on opacity {
					NumberAnimation { duration: 150 }
				}
			}
		}

		Item {
			id: dataRefreshPadding

			Layout.fillWidth: true
			Layout.fillHeight: true

			visible: false
		}

		TableView {
			id: tableViewInternal

			Layout.fillWidth: true
			Layout.fillHeight: true

			focus: true
			clip: true
			boundsMovement: Flickable.StopAtBounds
			interactive: !quickSettings.hovered
			rowSpacing: 1

			contentWidth: root.tableContentWidth

			columnWidthProvider: tableHeader.columnWidthProvider

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
				}
			}

			delegate: Rectangle {

				implicitHeight: root.Material.delegateHeight

				property bool highlighted: root.indexIsHighlighted(row, column, root.highlightedRow, root.highlightedColumn)

				IzText {
					anchors {
						fill: parent
						leftMargin: 2
						rightMargin: 2
					}

					text: displayData !== undefined ? displayData : ""
				}

				MouseArea {
					id: mouseArea

					anchors.fill: parent

					hoverEnabled: true

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
				}

				color: highlighted ? Qt.tint(root.baseColor, Qt.rgba(root.tintColor.r, root.tintColor.g, root.tintColor.b, 0.256))
									: isSelected ? Qt.tint(root.baseColor, Qt.rgba(root.tintColor.r, root.tintColor.g, root.tintColor.b, 0.512))
									: (root.alternatingRowColors
												 ? (row % 2 !== 0
													? Qt.lighter(root.baseColor, root.lighterFactor)
													: Qt.darker(root.baseColor, root.darkerFactor))
												 : root.baseColor)
			}

			model: root.model

			Connections {
				target: root.model.source

				onRefreshStarted: {
					// WARNING: hack
					tableViewInternal.contentX = 0;
					tableViewInternal.contentY = 0;
				}
			}
		}

		IzBoxContainer {
			id: tableFooter

			Layout.preferredHeight: 30
			Layout.fillWidth: true

			RowLayout {
				anchors {
					fill: parent
					leftMargin: 4
					rightMargin: 4
				}

				IzText {
					Layout.fillHeight: true
					Layout.preferredWidth: 120

					text: qsTr("Załadowane: ") + root.loadedRows
				}

				IzText {
					Layout.fillHeight: true
					Layout.preferredWidth: 120

					text: qsTr("Zaznaczone: ") + root.selectionCount
				}

				IzText {
					Layout.fillHeight: true
					Layout.preferredWidth: 120

					text: qsTr("Wiersz: ") + root.currentRow
				}

				IzText {
					Layout.fillHeight: true
					Layout.preferredWidth: 120

					text: qsTr("Kolumna: ") + root.currentColumn
				}

				Item {
					Layout.fillHeight: true
					Layout.fillWidth: true
				}

				IzButton {
					Layout.preferredHeight: 26
					Layout.preferredWidth: 26

					fontIcon: "\uf450"
					tooltip: qsTr("Odśwież dane")
					enabled: root.model.source.queryIsValid

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
					tooltip: qsTr("Eksportuj dane do Excel'a")

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
					tooltip: tableViewInternal.activeFocus ? qsTr("Nawigacja klawiaturą aktywna") : qsTr("Nawigacja klawiaturą nieaktywna")

					onReleased: {
						tableViewInternal.forceActiveFocus();
					}
				}
			}
		}
	}
}
