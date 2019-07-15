import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import Qt.labs.platform 1.0
import IzSQLTableView 1.0
import IzLibrary 1.0

Control {
	id: root

	property real darkerFactor
	property real lighterFactor
	property color baseColor
	property color tintColor
	property bool alternatingRowColors
	property bool filtersVisible
	property int selectionMode

	signal resetSettings()

	hoverEnabled: true
	state: "hidden"
	implicitWidth: 350
	padding: 1

	background: Rectangle {
		color: Material.background
		border.color: Material.accent
		border.width: 1
		opacity: root.hovered ? 1.0 : 0.18

		MouseArea {
			anchors.fill: parent
			preventStealing: true
			propagateComposedEvents: false
		}
	}

	ColorDialog {
		id: colorDialog

		property bool isSelectingBaseColor: false
		property color oldColor

		// used to open color dialog
		function openColorDialog(isSelectingBaseColor) {
			colorDialog.isSelectingBaseColor = isSelectingBaseColor;
			if (isSelectingBaseColor) {
				colorDialog.currentColor = root.baseColor;
				colorDialog.oldColor = root.baseColor;
			} else {
				colorDialog.currentColor = root.tintColor;
				colorDialog.oldColor = root.tintColor;
			}
			colorDialog.open();
		}

		onCurrentColorChanged: {
			if (isSelectingBaseColor) {
				root.baseColor = currentColor;
			} else {
				root.tintColor = currentColor;
			}
		}

		onAccepted: {
			if (isSelectingBaseColor) {
				root.baseColor = color;
			} else {
				root.tintColor = color;
			}
		}

		onRejected: {
			if (isSelectingBaseColor) {
				root.baseColor = colorDialog.oldColor;
			} else {
				root.tintColor = colorDialog.oldColor;
			}
		}
	}

	contentItem: ColumnLayout {
		id: mainLayout

		opacity: root.hovered ? 1.0 : 0.18
		spacing: 0

		RowLayout {
			Layout.fillWidth: true
			Layout.minimumHeight: 30
			Layout.maximumHeight: 30

			IzText {
				Layout.fillWidth: true
				Layout.leftMargin: 4
				Layout.alignment: Qt.AlignLeft

				text: qsTr("View settings")
				font.bold: true
			}

			IzButton {
				id: showCloseButton

				Layout.preferredHeight: 26
				Layout.preferredWidth: 26
				Layout.alignment: Qt.AlignRight

				fontIcon: root.state === "shown" ? "\uf140" : "\uf143"
				tooltip: root.state === "shown" ? qsTr("Show options") : qsTr("Hide options")

				onReleased: {
					if (root.state === "shown") {
						root.state = "hidden";
					} else {
						root.state = "shown";
					}
				}
			}
		}

		GridLayout {
			id: internalLayout

			Layout.fillWidth: true
			Layout.fillHeight: true
			Layout.margins: 4

			columns: 4

			Item {
				Layout.fillWidth: true
				Layout.preferredHeight: 24

				IzText {
					anchors.fill: parent

					text: qsTr("Base color:")
				}
			}

			Item {
				Layout.fillWidth: true
				Layout.preferredHeight: 24

				Rectangle {
					anchors.fill: parent

					color: root.baseColor
					border.color: root.Material.accent
					border.width: 1

					MouseArea {
						anchors.fill: parent

						onDoubleClicked: {
							colorDialog.openColorDialog(true);
						}
					}
				}
			}

			Item {
				Layout.fillWidth: true
				Layout.preferredHeight: 24

				IzText {
					anchors.fill: parent

					text: qsTr("Selection:")
				}
			}

			Item {
				Layout.fillWidth: true
				Layout.preferredHeight: 24

				Rectangle {
					anchors.fill: parent

					color: root.tintColor
					border.color: root.Material.accent
					border.width: 1

					MouseArea {
						anchors.fill: parent

						onDoubleClicked: {
							colorDialog.openColorDialog(false);
						}
					}
				}
			}

			Item {
				Layout.fillWidth: true
				Layout.preferredHeight: 24

				IzText {
					anchors.fill: parent

					text: qsTr("Dark factor:")
				}
			}

			Item {
				Layout.fillWidth: true
				Layout.preferredHeight: 24

				QuickSettingsSpinBox {
					anchors.fill: parent

					value: root.darkerFactor * 100;

					onRealValueChanged: {
						root.darkerFactor = value / 100;
					}
				}
			}

			Item {
				Layout.fillWidth: true
				Layout.preferredHeight: 24

				IzText {
					anchors.fill: parent

					text: qsTr("Light factor:")
				}
			}

			Item {
				Layout.fillWidth: true
				Layout.preferredHeight: 24

				QuickSettingsSpinBox {
					anchors.fill: parent

					value: root.lighterFactor * 100;

					onRealValueChanged: {
						root.lighterFactor = realValue;
					}
				}
			}

			Item {
				Layout.fillWidth: true
				Layout.preferredHeight: 24

				IzText {
					anchors.fill: parent

					text: qsTr("Alt. rows:")
				}
			}

			Item {
				Layout.fillWidth: true
				Layout.preferredHeight: 24

				IzCheckBox {
					anchors.fill: parent

					checked: root.alternatingRowColors

					onClicked: {
						root.alternatingRowColors = !root.alternatingRowColors;
					}
				}
			}

			Item {
				Layout.fillWidth: true
				Layout.preferredHeight: 24

				IzText {
					anchors.fill: parent

					text: qsTr("Filters:")
				}
			}

			Item {
				Layout.fillWidth: true
				Layout.preferredHeight: 24

				IzCheckBox {
					anchors.fill: parent

					checked: root.filtersVisible

					onClicked: {
						root.filtersVisible = !root.filtersVisible;
					}
				}
			}

			Item {
				Layout.fillWidth: true
				Layout.preferredHeight: 24

				IzText {
					anchors.fill: parent

					text: qsTr("Zaznaczanie:")
				}
			}

			Item {
				Layout.fillWidth: true
				Layout.preferredHeight: 24

				IzComboBox {
					anchors.fill: parent

					textRole: "description"

					model: ListModel {
						ListElement {
							description: qsTr("Rows")
							modeValue: IzSQLTableViewEnums.SelectRows
						}

						ListElement {
							description: qsTr("Columns")
							modeValue: IzSQLTableViewEnums.SelectColumns
						}

						ListElement {
							description: qsTr("Cells")
							modeValue: IzSQLTableViewEnums.SelectCells
						}
					}

					onActivated: {
						root.selectionMode = model.get(currentIndex).modeValue;
					}

					Component.onCompleted: {
						for (var i = 0; i < model.count; ++i) {
							if (model.get(i).modeValue === root.selectionMode) {
								this.currentIndex = i;
								return;
							}
						}
					}
				}
			}

			Item {
				Layout.fillWidth: true
				Layout.preferredHeight: 24
			}

			Item {
				Layout.fillWidth: true
				Layout.preferredHeight: 24

				TableButton {
					anchors.fill: parent

					text: qsTr("Defaults")

					onReleased: {
						root.resetSettings();
					}
				}
			}
		}
	}

	states: [
		State {
			name: "hidden"

			PropertyChanges {
				target: internalLayout
				visible: false
			}
		},
		State {
			name: "shown"

			PropertyChanges {
				target: internalLayout
				visible: true
			}
		}
	]
}
