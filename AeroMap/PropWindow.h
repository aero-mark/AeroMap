#ifndef PROPWINDOW_H
#define PROPWINDOW_H

#include <QtGui>

#include "ui_prop.h"

class PropWindow : public QWidget,
				   private Ui::PropForm
{
	Q_OBJECT

public:

	explicit PropWindow(QWidget* parent = nullptr);
	~PropWindow();

public slots:

	void OnToolChange();

private slots:

	void OnItemClicked(QTreeWidgetItem* item, int column);
	void OnApplyClicked();

private:

	enum class PropId
	{
		None,
		Depth,		// tool depth, meters
		Diameter,	// tool diameter, data points
	};

	QTreeWidgetItem* mp_ItemTool;

private:

	void LoadProperties();
};

#endif // #ifndef PROPWINDOW_H
