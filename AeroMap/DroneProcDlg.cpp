// DroneProcDlg.cpp
// Drone photogrammetry options.
//

#include "AeroMap.h"			// application header
#include "DroneProcDlg.h"

#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>

DroneProcDlg::DroneProcDlg(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);
	// make dialog fixed size
	setFixedSize(size());
	// remove question mark from title bar
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	cboInitStage->clear();
	cboInitStage->addItem("Load Dataset", Stage::Id::Dataset);
	cboInitStage->addItem("OpenSFM", Stage::Id::OpenSFM);
	cboInitStage->addItem("OpenMVS", Stage::Id::OpenMVS);
	cboInitStage->addItem("Filter Points", Stage::Id::Filter);
	cboInitStage->addItem("Mesh", Stage::Id::Mesh);
	cboInitStage->addItem("Texture", Stage::Id::Texture);
	cboInitStage->addItem("Georeference", Stage::Id::Georeference);
	cboInitStage->addItem("DEM", Stage::Id::DEM);
	cboInitStage->addItem("Orthophoto", Stage::Id::Orthophoto);
	cboInitStage->addItem("Report", Stage::Id::Report);

	// load defaults
	spinDemResolution->setValue(arg.dem_resolution);
	spinDemGapFillSteps->setValue(arg.dem_gapfill_steps);
	spinOrthoResolution->setValue(arg.ortho_resolution);
	chkDSM->setChecked(arg.dsm);
	chkDTM->setChecked(arg.dtm);

	// signals and slots
	verify_connect(cmdRun, SIGNAL(clicked()), this, SLOT(OnRun()));
	verify_connect(cmdCancel, SIGNAL(clicked()), this, SLOT(OnClose()));
}

DroneProcDlg::~DroneProcDlg()
{
}

Stage::Id DroneProcDlg::GetInitStage()
{
	return (Stage::Id)cboInitStage->currentData().toInt();
}

void DroneProcDlg::OnRun()
{
	arg.dem_resolution = spinDemResolution->value();
	arg.dem_gapfill_steps = spinDemGapFillSteps->value();
	arg.ortho_resolution = spinOrthoResolution->value();
	arg.dsm = chkDSM->isChecked();
	arg.dtm = chkDTM->isChecked();

    accept();
}

void DroneProcDlg::OnClose()
{
    reject();
}
