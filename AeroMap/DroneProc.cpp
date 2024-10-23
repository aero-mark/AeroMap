// DroneProc.cpp
// Drone photogrammetry pipeline manager.
//

#include <stdio.h>
#include <assert.h>

#include <QDir>
#include <QString>

#include "AeroMap.h"
#include "Logger.h"			// message logging facility
#include "Calc.h"
#include "DroneProc.h"

DroneProc::DroneProc(QObject* parent)
	: QObject(parent)
	, mp_StageDataset(new StageDataset())
	, mp_StageSFM(new StageSFM())
	, mp_StageMVS(new StageMVS())
	, mp_StageFilter(new StageFilter())
	, mp_StageMesh(new StageMesh())
	, mp_StageTexture(new StageTexture())
	, mp_StageGeoref(new StageGeoref())
	, mp_StageDEM(new StageDEM())
	, mp_StageOrtho(new StageOrtho())
	, mp_StageReport(new StageReport())
{
}

DroneProc::~DroneProc()
{
	delete mp_StageDataset;
	delete mp_StageSFM;
	delete mp_StageMVS;
	delete mp_StageFilter;
	delete mp_StageMesh;
	delete mp_StageTexture;
	delete mp_StageGeoref;
	delete mp_StageDEM;
	delete mp_StageOrtho;
	delete mp_StageReport;
}

int DroneProc::Run(Stage::Id initStage)
{
	int status = 0;
	
	AeroLib::InitRunLog();

	if (GetProject().LoadImageList() == 0)
	{
		GetApp()->LogWrite("No images found.");
		return -1;
	}

	if (initStage <= Stage::Id::Dataset)
		status = mp_StageDataset->Run();

	if ((status == 0) && (initStage <= Stage::Id::OpenSFM))
		status = mp_StageSFM->Run();

	if ((status == 0) && (initStage <= Stage::Id::OpenMVS))
		status = mp_StageMVS->Run();

	if ((status == 0) && (initStage <= Stage::Id::Filter))
		status = mp_StageFilter->Run();

	if ((status == 0) && (initStage <= Stage::Id::Mesh))
		status = mp_StageMesh->Run();

	if ((status == 0) && (initStage <= Stage::Id::Texture))
		status = mp_StageTexture->Run();

	if ((status == 0) && (initStage <= Stage::Id::Georeference))
		status = mp_StageGeoref->Run();

	if ((status == 0) && (initStage <= Stage::Id::DEM))
		status = mp_StageDEM->Run();

	if ((status == 0) && (initStage <= Stage::Id::Orthophoto))
		status = mp_StageOrtho->Run();

	if ((status == 0) && (initStage <= Stage::Id::Report))
		status = mp_StageReport->Run();

	GetApp()->LogWrite("Drone image processing complete.");

	return status;
}
