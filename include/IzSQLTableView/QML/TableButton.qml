import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import IzLibrary 1.0

Button {
	id: root

	topInset: 0
	bottomInset: 0
	leftInset: 0
	rightInset: 0
	flat: true

	contentItem: IzText {
		anchors.fill: root
		verticalAlignment: Text.AlignVCenter
		horizontalAlignment: Text.AlignHCenter
		text: root.text
	}
}
