// PropLabelDlg.cpp
// Property dialog for map labels.
//
// Author: Mark Stevens
// e-mail: xmark905@gmail.com
//

#include "PropLabelDlg.h"

#include <QtGui>
#include <QMessageBox>
#include <QColorDialog>

PropLabelDlg::PropLabelDlg(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);
	// make dialog fixed size
	setFixedSize(size());
	// remove question mark from title bar
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	// signals and slots
	verify_connect(cmdOK, SIGNAL(clicked()), this, SLOT(OnOK()));
	verify_connect(cmdCancel, SIGNAL(clicked()), this, SLOT(OnCancel()));
}

PropLabelDlg::~PropLabelDlg()
{
}

void PropLabelDlg::OnOK()
{
	accept();
}

void PropLabelDlg::OnCancel()
{
	reject();
}
