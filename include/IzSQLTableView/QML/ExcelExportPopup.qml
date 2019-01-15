import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12
import IzLibrary 1.0

Popup {
	id: root

	property string filePath
	property bool preserveColumnWidths: true
	property bool addAutofilters: true
	property bool selectedRowsOnly: false

	signal exportData(string filePath, bool preserveColumnWidths, bool addAutofilters, bool selectedRowsOnly)

	onClosed: {
		root.destroy();
	}

	// EXCEL export file dialog
	IzFileDialog {
		id: excelExportFileDialog

		saveMode: true
		nameFilters: [
			"Pliki XLSX (*.xlsx)"
		]

		onAccepted: {
			root.filePath = files[0];
		}
	}

	contentItem: ColumnLayout {
		RowLayout {
			Layout.preferredHeight: 20
			Layout.preferredWidth: 240

			Item {
				Layout.fillWidth: true
				Layout.fillHeight: true

				IzText {
					anchors.fill: parent

					text: qsTr("Zachowaj szerokości kolumn")
				}
			}

			Item {
				Layout.fillHeight: true
				Layout.preferredWidth: 30

				IzCheckBox {
					anchors.fill: parent

					checked: root.preserveColumnWidths

					onClicked: {
						root.preserveColumnWidths = !root.preserveColumnWidths;
					}
				}
			}
		}

		RowLayout {
			Layout.preferredHeight: 20
			Layout.preferredWidth: 240

			Item {
				Layout.fillWidth: true
				Layout.fillHeight: true

				IzText {
					anchors.fill: parent

					text: qsTr("Utwórz filtry danych")
				}
			}

			Item {
				Layout.fillHeight: true
				Layout.preferredWidth: 30

				IzCheckBox {
					anchors.fill: parent

					checked: root.addAutofilters

					onClicked: {
						root.addAutofilters = !root.addAutofilters;
					}
				}
			}
		}

		RowLayout {
			Layout.preferredHeight: 20
			Layout.preferredWidth: 240

			Item {
				Layout.fillWidth: true
				Layout.fillHeight: true

				IzText {
					anchors.fill: parent

					text: qsTr("Zaznaczone wiersze")
				}
			}

			Item {
				Layout.fillHeight: true
				Layout.preferredWidth: 30

				IzCheckBox {
					anchors.fill: parent

					checked: root.selectedRowsOnly

					onClicked: {
						root.selectedRowsOnly = !root.selectedRowsOnly;
					}
				}
			}
		}

		IzText {
			Layout.preferredHeight: 20
			Layout.preferredWidth: 240

			text: qsTr("<b>Plik:</b> ") + root.filePath
			enabled: true

			MouseArea {
				anchors.fill: parent
				hoverEnabled: true

				ToolTip.visible: containsMouse && root.filePath !== ""
				ToolTip.text: root.filePath
			}
		}

		RowLayout {
			Layout.preferredHeight: 26
			Layout.fillWidth: true

			Item {
				Layout.fillHeight: true
				Layout.fillWidth: true

				TableButton {
					anchors.fill: parent
					text: qsTr("Wybierz ściezkę")

					onReleased: {
						excelExportFileDialog.open();
					}
				}
			}

			Item {
				Layout.fillHeight: true
				Layout.fillWidth: true

				TableButton {
					anchors.fill: parent
					visible: root.filePath !== ""
					text: qsTr("Eksportuj")

					onReleased: {
						root.exportData(root.filePath,
										root.preserveColumnWidths,
										root.addAutofilters,
										root.selectedRowsOnly);
						root.close();
					}
				}
			}
		}
	}
}
