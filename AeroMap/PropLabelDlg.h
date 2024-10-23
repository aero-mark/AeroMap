#ifndef PROPLABELDLG_H
#define PROPLABELDLG_H

#include "MapEdit.h"			// application header

#include <QDialog>
#include <QGraphicsScene>
#include "ui_prop_label.h"

class PropLabelDlg : public QDialog,
				     private Ui::PropLabelDlg
{
	Q_OBJECT

public:

	explicit PropLabelDlg(QWidget* parent = nullptr);
	~PropLabelDlg();

	XString GetText() { return txtText->text(); }
	void    SetText(XString text) { txtText->setText(text.c_str()); }

private slots:

	void OnOK();
	void OnCancel();
};

#endif // #ifndef PROPLABELDLG_H
