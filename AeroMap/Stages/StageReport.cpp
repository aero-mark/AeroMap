// StageReport.cpp
// Generate PDF report.
//

#include <QPdfWriter>

#include "StageReport.h"

const int OFFSETV = 240;		// default line spacing

int StageReport::Run()
{
	// Inputs:
	// Outputs:
	//		+ odm_report
	//			report.pdf
	//

	int status = 0;

	GetApp()->LogWrite("Report...");
	BenchmarkStart();

	AeroLib::CreateFolder(tree.odm_report);

	XString file_name = XString::CombinePath(tree.odm_report, "report.pdf");
	QFile outputFile(file_name.c_str());
	if (!outputFile.open(QIODevice::WriteOnly))
	{
		Logger::Write(__FUNCTION__, "Unable to create '%s'", file_name.c_str());
		assert(false);
		return -1;
	}

	mp_Writer = new QPdfWriter(&outputFile);
	mp_Writer->setPageSize(QPageSize(QPageSize::Letter));
	mp_Writer->setPageOrientation(QPageLayout::Portrait);
	m_dpi = mp_Writer->resolution();

	mp_Painter = new QPainter(mp_Writer);

	SetupFonts();

	OutputSummary();
	OutputOrthophoto();
	OutputParameters();

	delete mp_Painter;

	outputFile.close();
	delete mp_Writer;

	BenchmarkStop("Report");

	return status;
}

void StageReport::OutputSummary()
{
	OutputHeader(false);

	mp_Painter->setFont(m_FontHeader1);
	mp_Painter->drawText(800, 1000, "Summary");

	mp_Painter->setFont(m_FontBody);
	int y = 1400;
	mp_Painter->drawText(1600, y += OFFSETV, "Date Processed: ---");
	mp_Painter->drawText(1600, y += OFFSETV, "Date Collected: ---");
	mp_Painter->drawText(1600, y += OFFSETV, "Area Covered: ---");
}

void StageReport::OutputOrthophoto()
{
	if (AeroLib::FileExists(tree.odm_orthophoto_tif) == false)
		return;

	QImage image;
	if (image.load(tree.odm_orthophoto_tif.c_str()) == false)
	{
		Logger::Write(__FUNCTION__, "Unable to load orthophoto: '%s'", tree.odm_orthophoto_tif.c_str());
		return;
	}

	int MAX_IMAGE_SIZE = 800;
	int w = image.width();
	int h = image.height();
	int max_dim = (w > h ? w : h);
	if (max_dim > MAX_IMAGE_SIZE)
	{
		// since image is stored in pdf - and loaded every time
		// report is viewed - scale down to a reasonable size
		double sf = (double)MAX_IMAGE_SIZE / (double)max_dim;
		image = image.scaled(w * sf, h * sf, Qt::AspectRatioMode::KeepAspectRatio);
	}

	XString ortho_file = XString::CombinePath(tree.odm_report, "ortho.png");
	if (image.save(ortho_file.c_str(), "PNG") == false)
	{
		Logger::Write(__FUNCTION__, "Unable to write orthophoto: '%s'", ortho_file.c_str());
		return;
	}

	OutputHeader();

	mp_Painter->setFont(m_FontHeader1);
	mp_Painter->drawText(800, 1000, "Orthophoto");

	// desired size of image, inches
	const double WIDTH_IN = 6.5;
	double height_in = ((double)image.height() / (double)image.width()) * WIDTH_IN;

	QRect rectTarget(1000, 1600, m_dpi * WIDTH_IN, m_dpi * height_in);
	QRect rectSrc = image.rect();
	mp_Painter->drawImage(rectTarget, image, rectSrc, Qt::AutoColor);
}

void StageReport::OutputParameters()
{
	OutputHeader();

	mp_Painter->setFont(m_FontHeader1);
	mp_Painter->drawText(800, 1000, "Parameters");

	mp_Painter->setFont(m_FontBody);
	int y = 1400;
	mp_Painter->drawText(1600, y += OFFSETV, XString::Format("DEM Resolution: %0.1f", arg.dem_resolution).c_str());
	mp_Painter->drawText(1600, y += OFFSETV, XString::Format("Orthophoto Resolution: %0.1f", arg.ortho_resolution).c_str());
	mp_Painter->drawText(1600, y += OFFSETV, XString::Format("DSM: %s", arg.dsm ? "True" : "False").c_str());
	mp_Painter->drawText(1600, y += OFFSETV, XString::Format("DTM: %s", arg.dtm ? "True" : "False").c_str());
}

void StageReport::OutputHeader(bool new_page)
{
	if (new_page)
		mp_Writer->newPage();

	mp_Painter->setFont(m_FontHeader);
	mp_Painter->drawText(400, 400, "AeroMap Report");
}

void StageReport::SetupFonts()
{
	m_FontHeader.setFamily("Cambria");
	m_FontHeader.setPointSize(12.0);
	m_FontHeader.setBold(true);
	m_FontHeader.setItalic(false);

	m_FontHeader1.setFamily("Cambria");
	m_FontHeader1.setPointSize(11.0);
	m_FontHeader1.setBold(true);
	m_FontHeader1.setItalic(false);

	m_FontBody.setFamily("Consolas");
	m_FontBody.setPointSize(10.0);
	m_FontBody.setBold(false);
	m_FontBody.setItalic(false);
}
