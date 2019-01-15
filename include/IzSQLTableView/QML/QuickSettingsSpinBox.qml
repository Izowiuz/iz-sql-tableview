import QtQuick 2.12
import QtQuick.Controls 2.12
import IzLibrary 1.0

SpinBox {
	id: root

	property int decimals: 2
	property real realValue: root.value / 100

	from: 0
	to: 100 * 100
	stepSize: 10
	implicitWidth: 60

	validator: DoubleValidator {
		bottom: Math.min(root.from, root.to)
		top: Math.max(root.from, root.to)
	}

	textFromValue: function(value, locale) {
		return Number(value / 100).toLocaleString(locale, 'f', root.decimals);
	}

	valueFromText: function(text, locale) {
		return Number.fromLocaleString(locale, text) * 100;
	}

	contentItem: IzText {
		anchors.fill: parent
		horizontalAlignment: Text.AlignHCenter
		text: root.realValue > 0.0 ? root.realValue : qsTr("Wył.")
	}

	background: Item {}
}
