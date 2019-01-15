#ifndef IZSQLTABLEVIEWPLUGIN_ENUMS_H
#define IZSQLTABLEVIEWPLUGIN_ENUMS_H

#include <QtCore>
#include <QItemSelectionModel>

// WARNING: nazwy wartości enumów muszą być unikalne :O
namespace IzSQLTableView
{
	Q_NAMESPACE

	enum class TableSelectionMode : int {
		// select single cells
		SelectCells   = 1,
		// select whole rows
		SelectRows    = 2,
		// select whole columns
		SelectColumns = 4
	};
	Q_ENUM_NS(TableSelectionMode)

	// sort types
	enum class SortType : uint8_t {
		// column is unsorted
		Unsorted = 1,
		// column is sorted - ascending order
		Ascending,
		// column is sorted - descending order
		Descending
	};
	Q_ENUM_NS(SortType)

}   // namespace IzSQLTableView

#endif   // IZSQLTABLEVIEWPLUGIN_ENUMS_H
