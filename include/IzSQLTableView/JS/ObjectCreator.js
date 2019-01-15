.pragma library

function create(parent, objectURL, parameters) {
	if (parent === null) {
		console.warn("Error creating object: invalid parent.");
		return null;
	}
	var component = Qt.createComponent(objectURL);
	var object = component.createObject(
		parent,
		parameters
	);
	if (object === null) {
		console.warn("Error creating object.");
		return null;
	} else {
		return object;
	}
}
