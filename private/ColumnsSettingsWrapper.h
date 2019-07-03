#ifndef IZSQLTABLEVIEW_COLUMNSSETTINGSWRAPPER_H
#define IZSQLTABLEVIEW_COLUMNSSETTINGSWRAPPER_H

#include <QAbstractItemModel>

namespace IzSQLTableView
{
	class TableHeaderModel;

	// TODO: te coś to jakby mały hack
	class ColumnsSettingsWrapper : public QAbstractItemModel
	{
		Q_OBJECT
		Q_DISABLE_COPY(ColumnsSettingsWrapper)

	public:
		// ctor
		explicit ColumnsSettingsWrapper(TableHeaderModel* headerModel);

		// basic functionality
		QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
		QModelIndex parent(const QModelIndex& index) const override;
		int rowCount(const QModelIndex& parent = QModelIndex()) const override;
		int columnCount(const QModelIndex& parent = QModelIndex()) const override;

		// data
		QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

		// role names
		QHash<int, QByteArray> roleNames() const;

	private:
		// handler to the table header model
		TableHeaderModel* m_header;
	};
}   // namespace IzSQLTableView

#endif   // IZSQLTABLEVIEW_COLUMNSSETTINGSWRAPPER_H
