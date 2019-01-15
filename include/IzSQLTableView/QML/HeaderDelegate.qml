import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12
import IzLibrary 1.0
import IzSQLTableView 1.0

Button {
	id: root

	// handler on the view
	property SQLTableViewImpl tableView: null

	// dynamic resize marker
	property QtObject marker: null

	// emited when column size was set
	signal setColumnWidth(int column, int width)

	// emited when colum filter was requested
	signal filterColumn(int column, string filter)

	// emited when colum sort was requested
	signal sortColumn(int column)

	// if true, field for filter will be visible
	property bool filterFieldVisible: false

	text: displayData
	hoverEnabled: true
	flat: true
	padding: 0
	topInset: 0
	bottomInset: 0
	leftInset: 0
	rightInset: 0

	background: Item {}

	contentItem: ColumnLayout {
		spacing: 0

		RowLayout {
			Layout.minimumHeight: 30
			Layout.maximumHeight: 30
			Layout.fillWidth: true
			Layout.leftMargin: 7
			Layout.rightMargin: 9

			IzText {
				id: headerText

				Layout.fillWidth: true
				Layout.fillHeight: true

				text: root.text
				font.bold: true

				Behavior on color {
					ColorAnimation {
						duration: 150
					}
				}
			}

			IzButton {
				Layout.preferredHeight: 18
				Layout.preferredWidth: 18

				visible: isFiltered
				fontIcon: "\uf235"
				tooltip: qsTr("Usuń filtr")

				onReleased: {
					root.filterColumn(index, "");
				}
			}
		}

		IzTextField {
			id: filterTextField

			Layout.preferredHeight: 23
			Layout.fillWidth: true
			Layout.leftMargin: 7
			Layout.rightMargin: 9

			visible: root.filterFieldVisible

			placeholderText: root.text + "\u2026"
			text: filterValue

			onEditingFinished: {
				root.filterColumn(index, text);
			}
		}

		Item {
			Layout.fillHeight: true
			Layout.fillWidth: true
			Layout.leftMargin: 7
			Layout.rightMargin: 9
		}
	}

	onReleased: {
		if (!filterTextField.activeFocus) {
			root.sortColumn(index);
		}
	}

	Rectangle {
		id: sortIndicator

		anchors {
			left: parent.left
			leftMargin: 2
			right: parent.right
			rightMargin: 4
		}

		states: [
			State {
				name: "noSort"
				when: sortType === IzSQLTableViewEnums.Unsorted

				AnchorChanges {
					target: sortIndicator
					anchors.bottom: undefined
					anchors.top: undefined
				}
			},

			State {
				name: "ascendingSort"
				when: sortType === IzSQLTableViewEnums.Ascending

				AnchorChanges {
					target: sortIndicator
					anchors.bottom: undefined
					anchors.top: sortIndicator.parent.top
				}
			},

			State {
				name: "descendingSort"
				when: sortType === IzSQLTableViewEnums.Descending

				AnchorChanges {
					target: sortIndicator
					anchors.bottom: sortIndicator.parent.bottom
					anchors.top: undefined
				}
			}
		]

		height: 1
		color: root.Material.accent
		visible: sortType !== IzSQLTableViewEnums.Unsorted
	}

	Rectangle {
		id: columnResizer

		anchors {
			top: parent.top
			topMargin: 2
			bottom: parent.bottom
			bottomMargin: 2
			right: parent.right
		}

		width: resizeMouseArea.containsMouse ? 2 : 1
		color: root.Material.accent
		opacity: resizeMouseArea.containsMouse || resizeMouseArea.drag.active
				 ? 1.0
				 : 0.23

		Behavior on opacity {
			NumberAnimation {
				duration: 100
			}
		}

		Behavior on height {
			NumberAnimation {
				duration: 50
			}
		}

		MouseArea {
			id: resizeMouseArea

			anchors.fill: parent

			cursorShape: Qt.SizeHorCursor
			hoverEnabled: true
			drag {
				target: columnResizer
				minimumX: 20
				smoothed: false
			}

			onPressed: {
				columnResizer.anchors.right = undefined;
				if (root.marker !== null) {
					root.marker.destroy();
				}
				var mrk = resizeMarker.createObject(columnResizer);
				root.marker = mrk;
				mrk.open();
			}

			onReleased: {
				if (columnResizer.x > 0) {
					root.setColumnWidth(index, columnResizer.x);
				}
				columnResizer.anchors.right = columnResizer.parent.right;
				if (root.marker !== null) {
					root.marker.destroy();
				}
			}
		}
	}

	Component {
		id: resizeMarker

		Popup {
			y: columnResizer.y + root.height - 6

			height: root.tableView.view.height + 6
			width: 2

			background: Rectangle {
				color: root.Material.accent
			}

			contentItem: Item {}
		}
	}

	states: [
		State {
			name: "hovered"
			when: root.hovered && !root.pressed

			PropertyChanges {
				target: headerText
				color: root.Material.accent
			}
		},

		State {
			name: "pressed"
			when: root.pressed

			PropertyChanges {
				target: headerText
				color: Qt.darker(root.Material.accent)
			}
		}
	]
}
