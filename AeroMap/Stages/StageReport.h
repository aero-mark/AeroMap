#ifndef STAGEREPORT_H
#define STAGEREPORT_H

#include "Stage.h"

class StageReport : Stage
{
public:

	virtual int Run() override;

private:

	QFont m_FontHeader;
	QFont m_FontHeader1;
	QFont m_FontBody;

	QPdfWriter* mp_Writer;
	QPainter* mp_Painter;

	int m_dpi;

private:

	void SetupFonts();
	void OutputSummary();
	void OutputOrthophoto();
	void OutputParameters();
	void OutputHeader(bool new_page = true);
};

#endif // #ifndef STAGEREPORT_H
