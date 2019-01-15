﻿import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import IzLibrary 1.0

Popup {
	id: root

	property QtObject headerModel
	property string globalFilterValue

	signal changeColumnWidth(int column, int width)
	signal setGlobalColumnFilter(string filterValue)

	implicitWidth: 350
	implicitHeight: 500

	onClosed: {
		root.destroy();
	}

	ColumnLayout {
		anchors.fill: parent

		RowLayout {
			Layout.preferredHeight: 30
			Layout.fillHeight: false
			Layout.fillWidth: true

			Item {
				Layout.fillHeight: true
				Layout.fillWidth: true

				IzText {
					anchors.fill: parent

					horizontalAlignment: Qt.AlignHCenter
					font.bold: true
					text: qsTr("Kolumna")
				}
			}

			Item {
				Layout.fillHeight: true
				Layout.fillWidth: true

				IzText {
					anchors.fill: parent

					horizontalAlignment: Qt.AlignHCenter
					font.bold: true
					text: qsTr("Szerokość")
				}
			}

			Item {
				Layout.fillHeight: true
				Layout.fillWidth: true

				IzText {
					anchors.fill: parent

					horizontalAlignment: Qt.AlignHCenter
					font.bold: true
					text: qsTr("Filtr")
				}
			}

			Item {
				Layout.fillHeight: true
				Layout.fillWidth: true

				IzText {
					anchors.fill: parent

					horizontalAlignment: Qt.AlignHCenter
					font.bold: true
					text: qsTr("Widoczność")
				}
			}
		}

		ListView {
			Layout.fillHeight: true
			Layout.fillWidth: true

			boundsMovement: Flickable.StopAtBounds
			model: root.headerModel.source.modelWrapper()
			clip: true
			spacing: 5

			delegate: RowLayout {
				height: 24
				width: ListView.view.width

				Item {
					Layout.fillHeight: true
					Layout.fillWidth: true

					IzText {
						anchors.fill: parent

						text: displayData
					}
				}

				Item {
					Layout.fillHeight: true
					Layout.fillWidth: true

					IzTextField {
						anchors.fill: parent

						placeholderText: qsTr("Szerokość") + "\u2026"
						validator: IntValidator {
							bottom: 15
							top: 1200
						}
						text: columnWidth

						onEditingFinished: {
							root.headerModel.source.setData(0, index, text, "columnWidth");
						}
					}
				}

				Item {
					Layout.fillHeight: true
					Layout.fillWidth: true

					IzCheckBox {
						anchors.fill: parent

						checked: isFiltered === true
						enabled: isFiltered === true

						MouseArea {
							anchors.fill: parent

							propagateComposedEvents: false
							preventStealing: true

							onReleased: {
								root.headerModel.source.setData(0, index, "", "filterValue");
							}
						}
					}
				}

				Item {
					Layout.fillHeight: true
					Layout.fillWidth: true

					IzCheckBox {
						anchors.fill: parent

						checked: isVisible

						MouseArea {
							anchors.fill: parent

							propagateComposedEvents: false
							preventStealing: true

							onReleased: {
								root.headerModel.source.setData(0, index, !parent.checked, "isVisible");
							}
						}
					}
				}

			}
			ScrollBar.vertical: ScrollBar {
				minimumSize: 0.1
				width: 5
			}

			ScrollBar.horizontal: ScrollBar {
				minimumSize: 0.1
				height: 5
			}
		}

		IzGroupBox {
			Layout.fillWidth: true
			Layout.preferredHeight: 70

			title: qsTr("Filtr globalny kolumn: ") + root.globalFilterValue

			RowLayout {
				anchors {
					fill: parent
					margins: 2
				}

				IzTextInput {
					Layout.fillWidth: true
					Layout.fillHeight: true
					Layout.alignment: Qt.AlignCenter

					placeholderText: qsTr("Wyrażenie regularne") + "\u2026"
					selectByMouse: true

					onEditingFinished: {
						root.setGlobalColumnFilter(text);
					}
				}
			}

		}
	}
}
