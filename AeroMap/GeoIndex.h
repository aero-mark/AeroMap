#ifndef GEOINDEX_H
#define GEOINDEX_H

#include <QObject>
#include <QThread>

#include "XString.h"
#include "Calc.h"
#include "Gis.h"		// GIS support class

#include <vector>

class GeoIndex : public QObject
{
    Q_OBJECT

public:

	GeoIndex(const char* dataPath, QObject* parent = nullptr);
	~GeoIndex();

	unsigned int FindFiles(double latN, double latS, double lonE, double lonW, GIS::GEODATA dataType = GIS::GEODATA::None);
	unsigned int FindFiles(RectLL rectExtents, GIS::GEODATA dataType = GIS::GEODATA::None);
	unsigned int FindFiles(GIS::GEODATA dataType);
	unsigned int FindFiles(const char* stateID, GIS::GEODATA dataType = GIS::GEODATA::DEM10);
	const char* GetFoundFileName(unsigned int fileIdx);
	bool GetFoundFileExtents(RectLL& extents);								// extents of all found files
	bool GetFoundFileExtents(unsigned int ai_FileIdx, RectLL& extents);		// extents of one found file
	double GetFoundFilePitchX(unsigned int fileIdx);
	double GetFoundFilePitchY(unsigned int fileIdx);

private:

	struct IndexEntryType
	{
		double latN;			// latitude extents
		double latS;
		double lonE;			// longitude extents
		double lonW;
		GIS::GEODATA fileType;	// dted0, dem, etc.
		double pitchX;			// horizontal spacing between pixels
		double pitchY;
		char fileName[255];		// fully-qualified path/file name

		IndexEntryType()
			: latN(0.0)
			, latS(0.0)
			, lonE(0.0)
			, lonW(0.0)
			, pitchX(0.0)
			, pitchY(0.0)
			, fileType(GIS::GEODATA::None)
		{
			memset(fileName, 0, sizeof(fileName));
		}
	};

	XString ms_DataPath;		// root path of geodata / location of index file
	UInt32 m_EntryCount;		// on rebuild, # of files indexed
	int mi_IndexFile;			// index file handle
	std::vector<IndexEntryType> mv_FoundFiles;		// results of most recent "find" operation
	RectLL m_Extents;			// extents of found files

private:

	void RebuildIndex(const char* pathName, int fileHandle);
	double Round18(double v);

public slots:

	void RebuildIndex();

signals:

	void fileIndexed(QString path);
	void finished(unsigned int fileCount);
};

#endif // #ifndef GEOINDEX_H
