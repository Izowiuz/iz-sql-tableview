import QtQuick 2.12
import QtQuick.Controls 2.12
import IzLibrary 1.0

Control {
	id: root

	signal showColumnSettings()

	padding: 1
	opacity: root.hovered ? 1.0 : 0.18

	IzButton {
		id: showCloseButton

		anchors.fill: parent

		fontIcon: "\uf837"
		tooltip: qsTr("Column options")

		onReleased: {
			root.showColumnSettings();
		}
	}
}
