// GeoIndex.cpp
// Manage geospatial data index
//

#include <stdio.h>
#include <assert.h>
#include <io.h>

#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <QDir>
#include <QString>

#include "gdal_priv.h"
#include "cpl_conv.h"		// for CPLMalloc()
#include "ogrsf_frmts.h"

#include "Logger.h"			// message logging facility
#include "Calc.h"
#include "RasterFile.h"
#include "GeoIndex.h"

#define FILE_NAME "geodata.idx"

GeoIndex::GeoIndex(const char* pathName, QObject* parent)
	: QObject(parent)
	, ms_DataPath(pathName)
	, m_EntryCount(0)
{
	ms_DataPath.FormatPathName();
}

GeoIndex::~GeoIndex()
{
	mv_FoundFiles.clear();
}

unsigned int GeoIndex::FindFiles(double latN, double latS, double lonE, double lonW, GIS::GEODATA dataType /* = NONE */)
{
	// Find all index entries with files of given type within specified region
	//
	// Inputs:
	//		latN/latS = latitude extents
	//		lonE/lonW = longitude extents
	//		dataType  = file type filter or NONE for all types
	// Outputs:
	//		return == # of files found matching search criteria
	//

	mv_FoundFiles.clear();
	XString indexFile = XString::CombinePath(ms_DataPath.c_str(), FILE_NAME);

	int fileHandle = _open(indexFile.c_str(), O_RDONLY | O_BINARY);
	if (fileHandle < 1)
	{
		Logger::Write(__FUNCTION__, "Unable to open index file '%s' (%s)", indexFile.c_str(), strerror(errno));
		return 0;
	}

	RectD searchRect;
	searchRect.x0 = lonW; searchRect.x1 = lonE;
	searchRect.y0 = latN; searchRect.y1 = latS;

	IndexEntryType entry;
	while (_read(fileHandle, &entry, sizeof(entry)) == sizeof(entry))
	{
		if (GIS::GEODATA::None == dataType || entry.fileType == dataType)		// file type filter
		{
			RectD fileRect;
			fileRect.x0 = entry.lonW; fileRect.x1 = entry.lonE;
			fileRect.y0 = entry.latN; fileRect.y1 = entry.latS;

			if (IntersectRectRect(searchRect, fileRect))	// found
			{
				// update extents
				if (mv_FoundFiles.size() == 0)
				{
					m_Extents.latN = entry.latN;
					m_Extents.latS = entry.latS;
					m_Extents.lonE = entry.lonE;
					m_Extents.lonW = entry.lonW;
				}
				else
				{
					if (m_Extents.latN < entry.latN)
						m_Extents.latN = entry.latN;
					if (m_Extents.latS > entry.latS)
						m_Extents.latS = entry.latS;
					if (m_Extents.lonE < entry.lonE)
						m_Extents.lonE = entry.lonE;
					if (m_Extents.lonW > entry.lonW)
						m_Extents.lonW = entry.lonW;
				}

				// update found file list
				mv_FoundFiles.push_back(entry);
			}
		}
	}

	if (_close(fileHandle) != 0)
	{
		Logger::Write(__FUNCTION__, "Unable to close index file '%s' (%s)", indexFile.c_str(), strerror(errno));
		return 0;
	}

	return (unsigned int)mv_FoundFiles.size();
}

unsigned int GeoIndex::FindFiles(RectLL rectExtents, GIS::GEODATA dataType /* = NONE */)
{
	// convenience overload
	return FindFiles(rectExtents.latN, rectExtents.latS, rectExtents.lonE, rectExtents.lonW, dataType);
}

unsigned int GeoIndex::FindFiles(GIS::GEODATA dataType)
{
	// Find all index entries with files of given type
	//
	// Inputs:
	//		dataType  = file type filter
	// Outputs:
	//		return == # of files found matching search criteria
	//

	mv_FoundFiles.clear();
	XString indexFile = XString::CombinePath(ms_DataPath.c_str(), FILE_NAME);

	int fileHandle = _open(indexFile.c_str(), O_RDONLY | O_BINARY);
	if (fileHandle < 1)
	{
		Logger::Write(__FUNCTION__, "Unable to open index file '%s' (%s)", indexFile.c_str(), strerror(errno));
		return 0;
	}

	IndexEntryType entry;
	while (_read(fileHandle, &entry, sizeof(entry)) == sizeof(entry))
	{
		if (entry.fileType == dataType)		// file type filter
		{
			// update extents
			if (mv_FoundFiles.size() == 0)
			{
				m_Extents.latN = entry.latN;
				m_Extents.latS = entry.latS;
				m_Extents.lonE = entry.lonE;
				m_Extents.lonW = entry.lonW;
			}
			else
			{
				if (m_Extents.latN < entry.latN)
					m_Extents.latN = entry.latN;
				if (m_Extents.latS > entry.latS)
					m_Extents.latS = entry.latS;
				if (m_Extents.lonE < entry.lonE)
					m_Extents.lonE = entry.lonE;
				if (m_Extents.lonW > entry.lonW)
					m_Extents.lonW = entry.lonW;
			}

			// update found file list
			mv_FoundFiles.push_back(entry);
		}
	}

	if (_close(fileHandle) != 0)
	{
		Logger::Write(__FUNCTION__, "Unable to close index file '%s' (%s)", indexFile.c_str(), strerror(errno));
		return 0;
	}

	return (unsigned int)mv_FoundFiles.size();
}

unsigned int GeoIndex::FindFiles(const char* stateID, GIS::GEODATA dataType)
{
	// Find all DEM files for a given state
	//
	// Inputs:
	//		stateID  = 2-char state ID
	//		dataType = file type filter
	// Outputs:
	//		return == # of files found matching search criteria
	//

	// currently only supported types
	assert((dataType == GIS::GEODATA::DEM10) || (dataType == GIS::GEODATA::DEM30));

	mv_FoundFiles.clear();
	XString indexFile = XString::CombinePath(ms_DataPath.c_str(), FILE_NAME);

	int fileHandle = _open(indexFile.c_str(), O_RDONLY | O_BINARY);
	if (fileHandle < 1)
	{
		Logger::Write(__FUNCTION__, "Unable to open index file '%s' (%s)", indexFile.c_str(), strerror(errno));
		return 0;
	}

	IndexEntryType entry;
	while (_read(fileHandle, &entry, sizeof(entry)) == sizeof(entry))
	{
		if (entry.fileType == dataType)		// file type filter
		{
			XString filePath = entry.fileName;
			filePath.NormalizePath();
			filePath.Tokenize("/");
			XString id = filePath.GetToken(filePath.GetTokenCount()-2);

			if (id.CompareNoCase(stateID))	// state filter
			{
				// update extents
				if (mv_FoundFiles.size() == 0)
				{
					m_Extents.latN = entry.latN;
					m_Extents.latS = entry.latS;
					m_Extents.lonE = entry.lonE;
					m_Extents.lonW = entry.lonW;
				}
				else
				{
					if (m_Extents.latN < entry.latN)
						m_Extents.latN = entry.latN;
					if (m_Extents.latS > entry.latS)
						m_Extents.latS = entry.latS;
					if (m_Extents.lonE < entry.lonE)
						m_Extents.lonE = entry.lonE;
					if (m_Extents.lonW > entry.lonW)
						m_Extents.lonW = entry.lonW;
				}

				// update found file list
				mv_FoundFiles.push_back(entry);
			}
		}
	}

	if (_close(fileHandle) != 0)
	{
		Logger::Write(__FUNCTION__, "Unable to close index file '%s' (%s)", indexFile.c_str(), strerror(errno));
		return 0;
	}

	return (unsigned int)mv_FoundFiles.size();
}

bool GeoIndex::GetFoundFileExtents(RectLL& extents)
{
	// Extents of all found files
	//

	if (mv_FoundFiles.size() == 0)		// extents not valid
		return false;

	extents = m_Extents;

	return true;
}

bool GeoIndex::GetFoundFileExtents(unsigned int fileIdx, RectLL& extents)
{
	// Extents of one found file
	//

	if (fileIdx < mv_FoundFiles.size())
	{
		extents.latN = mv_FoundFiles[fileIdx].latN;
		extents.latS = mv_FoundFiles[fileIdx].latS;
		extents.lonE = mv_FoundFiles[fileIdx].lonE;
		extents.lonW = mv_FoundFiles[fileIdx].lonW;
		return true;
	}

	return false;
}

double GeoIndex::GetFoundFilePitchX(unsigned int fileIdx)
{
	double pitchX = -1.0;

	if (fileIdx < mv_FoundFiles.size())
		pitchX = mv_FoundFiles[fileIdx].pitchX;

	return pitchX;
}

double GeoIndex::GetFoundFilePitchY(unsigned int fileIdx)
{
	double pitchY = -1.0;

	if (fileIdx < mv_FoundFiles.size())
		pitchY = mv_FoundFiles[fileIdx].pitchX;

	return pitchY;
}

const char* GeoIndex::GetFoundFileName(unsigned int foundIndex)
{
	// return pointer to file name
	return mv_FoundFiles[foundIndex].fileName;
}

void GeoIndex::RebuildIndex()
{
	m_EntryCount = 0;
	XString indexFile = XString::CombinePath(ms_DataPath.c_str(), FILE_NAME);

	int fileHandle = _open(indexFile.c_str(), O_CREAT | O_BINARY | O_TRUNC | O_WRONLY, S_IWRITE | S_IREAD);
	if (fileHandle < 1)
	{
		Logger::Write(__FUNCTION__, "Unable to create file '%s' (%s)", indexFile.c_str(), strerror(errno));
		return;
	}

	RebuildIndex(ms_DataPath.c_str(), fileHandle);

	if (_close(fileHandle) != 0)
	{
		Logger::Write(__FUNCTION__, "Unable to close file '%s' (%s)", indexFile.c_str(), strerror(errno));
		return;
	}

	emit finished((unsigned int)m_EntryCount);
}

void GeoIndex::RebuildIndex(const char* pathName, int fileHandle)
{
	QDir dir(pathName);
	dir.setFilter(QDir::Files | QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);

	QFileInfoList list = dir.entryInfoList();
	for (int i = 0; i < list.size(); ++i)
	{
		QFileInfo fileInfo = list.at(i);

		XString fileName = fileInfo.fileName().toLatin1().constData();
		XString filePath = fileInfo.absoluteFilePath().toLatin1().constData();

		if (fileInfo.isDir())
		{
			RebuildIndex(filePath.c_str(), fileHandle);
		}
		else
		{
			IndexEntryType entry;
			memset(&entry, 0, sizeof(entry));
			entry.fileType = GIS::GEODATA::None;
			strncpy(entry.fileName, filePath.c_str(), sizeof(entry.fileName));

			if (fileName.EndsWithNoCase(".DEM"))
			{
				GDALDataset* pDS = (GDALDataset *)GDALOpen(filePath.c_str(), GA_ReadOnly);

				if (pDS == nullptr)
				{
					Logger::Write(__FUNCTION__, "Unable to open dataset: %s", filePath.c_str());
				}
				else
				{
					int sizeX = pDS->GetRasterXSize();
					int sizeY = pDS->GetRasterYSize();

					const OGRSpatialReference* pSRS = pDS->GetSpatialRef();
					if (pSRS != nullptr)
					{
						assert(pSRS->IsProjected() == true);
						assert(pSRS->IsGeographic() == false);

						double geoTransform[6] = { 0 };
						CPLErr err = pDS->GetGeoTransform(geoTransform);
						if (err != CPLErr::CE_None)
							Logger::Write(__FUNCTION__, "GDALDataset::GetGeoTransform() returned error code %d", err);

						double pixelSizeX = geoTransform[1];
						double pixelSizeY = geoTransform[5];

						entry.pitchX = pixelSizeX;
						entry.pitchY = pixelSizeY;

						if (pixelSizeX == 10.0)
							entry.fileType = GIS::GEODATA::DEM10;
						else if (pixelSizeX == 30.0)
							entry.fileType = GIS::GEODATA::DEM30;
						else
							Logger::Write(__FUNCTION__, "Unsupported DEM file resolution: %0.1f", pixelSizeX);

						double xw = geoTransform[0];  // X Origin (top left corner)
						double yn = geoTransform[3];  // Y Origin (top left corner)

						double xe = xw + static_cast<double>(sizeX) * pixelSizeX;
						double ys = yn + static_cast<double>(sizeY) * pixelSizeY;

						int zone = pSRS->GetUTMZone();
						GIS::Hemi hemi = ys >= 0.0 ? GIS::Hemi::N : GIS::Hemi::S;
						GIS::XYToLatLon_UTM(zone, hemi, xw, ys, entry.latS, entry.lonW);
						GIS::XYToLatLon_UTM(zone, hemi, xw, yn, entry.latN, entry.lonW);
						GIS::XYToLatLon_UTM(zone, hemi, xe, yn, entry.latN, entry.lonE);
						GIS::XYToLatLon_UTM(zone, hemi, xe, ys, entry.latS, entry.lonE);

						// round to 1/8ths of a degree (usgs 7.5m quads)
						entry.latN = Round18(entry.latN);
						entry.latS = Round18(entry.latS);
						entry.lonE = Round18(entry.lonE);
						entry.lonW = Round18(entry.lonW);
					}

					GDALClose(pDS);
				}
			}
			else if ((fileName.EndsWithNoCase(".DT0"))
			      || (fileName.EndsWithNoCase(".DT1"))
			      || (fileName.EndsWithNoCase(".DT2")))
			{
				GDALDataset* pDS = (GDALDataset *)GDALOpen(filePath.c_str(), GA_ReadOnly);
				if (pDS == nullptr)
				{
					Logger::Write(__FUNCTION__, "Unable to open dataset: %s", filePath.c_str());
				}
				else
				{
					int sizeX = pDS->GetRasterXSize();
					int sizeY = pDS->GetRasterYSize();

					const OGRSpatialReference* pSRS = pDS->GetSpatialRef();
					if (pSRS != nullptr)
					{
						assert(pSRS->IsProjected() == false);
						assert(pSRS->IsGeographic() == true);

						double geoTransform[6] = { 0 };
						CPLErr err = pDS->GetGeoTransform(geoTransform);
						if (err != CPLErr::CE_None)
							Logger::Write(__FUNCTION__, "GDALDataset::GetGeoTransform() returned error code %d", err);

						double pixelSizeX = geoTransform[1];
						double pixelSizeY = geoTransform[5];

						entry.pitchX = pixelSizeX;
						entry.pitchY = pixelSizeY;

						entry.fileType = GIS::GetFileType(filePath.c_str());

						// gdal puts the pixel right on the data point, so you end up with
						// 1/2 of the pixel size border all around
						// this might be ok, or may need to round to whole degrees
						entry.lonW = geoTransform[0];  // X Origin (top left corner)
						entry.latN = geoTransform[3];  // Y Origin (top left corner)
						entry.lonE = entry.lonW + static_cast<double>(sizeX) * pixelSizeX;
						entry.latS = entry.latN + static_cast<double>(sizeY) * pixelSizeY;
					}

					GDALClose(pDS);
				}
			}
			else if (fileName.EndsWithNoCase(".TIF") || fileName.EndsWithNoCase(".TIFF"))
			{
				RasterFile* pRaster = new RasterFile(filePath.c_str());

				if (pRaster->HasGeoTransform())
				{
					if (pRaster->IsGeographic())
					{
						entry.fileType = GIS::GEODATA::GeoTIFF;

						//entry.pitchX = pixelSizeX;
						//entry.pitchY = pixelSizeY;

						entry.lonW = pRaster->GetExtents().x0;
						entry.latN = pRaster->GetExtents().y0;
						entry.lonE = pRaster->GetExtents().x1;
						entry.latS = pRaster->GetExtents().y1;
					}
					else
					{
						Logger::Write(__FUNCTION__, "Unhandled GeoTIFF SRS type: %s", filePath.c_str());
					}
				}

				delete pRaster;
			}
			else if (fileName.EndsWithNoCase(".BT"))
			{
//TODO:
//use gdal
				//VtpFile* pVtpFile = new VtpFile(filePath.c_str());

				//// non-utm files are not handled
				//if (pVtpFile->IsUTM())
				//{
				//	double yn, ys, xw, xe;
				//	pVtpFile->GetExtents(yn, ys, xe, xw);

				//	int zone = pVtpFile->GetUtmZone();
				//	GIS::Hemi hemi = zone > 0 ? GIS::Hemi::N : GIS::Hemi::S;

				//	GIS::XYToLatLon_UTM(zone, hemi, xw, ys, entry.latS, entry.lonW);
				//	GIS::XYToLatLon_UTM(zone, hemi, xw, yn, entry.latN, entry.lonW);
				//	GIS::XYToLatLon_UTM(zone, hemi, xe, yn, entry.latN, entry.lonE);
				//	GIS::XYToLatLon_UTM(zone, hemi, xe, ys, entry.latS, entry.lonE);

				//	entry.fileType = GIS::GEODATA::VtpBT;
				//	pVtpFile->GetExtents(entry.latN, entry.latS, entry.lonE, entry.lonW);
				//}
				//delete pVtpFile;
			}

			if (entry.fileType != GIS::GEODATA::None)
			{
				if (_write(fileHandle, &entry, sizeof(entry)) == sizeof(entry))
				{
					Logger::Write(__FUNCTION__, "%s indexed (%lu)", entry.fileName, m_EntryCount);
					m_EntryCount++;
				}
				else
				{
					Logger::Write(__FUNCTION__, "write() failed for '%s'", entry.fileName);
				}

				emit fileIndexed(QString(entry.fileName));
			}
		}
	}
}

double GeoIndex::Round18(double v)
{
	// Round to 1/8th of a degree.
	//
	// This is specifically to clean up my float error calculating 
	// the extents of USGS 7.5m DEM files.
	//

	double value = v;
	double v_rnd = 0.0;
	int v_sign = sign(v);

	double fracpart = fabs(fmod(value, 1.0));
	v = fabs(v);

	if (fracpart <= 0.0625)
	{
		v_rnd = trunc(v);
	}
	else if (fracpart > 0.0625 && fracpart <= 0.1875)
	{
		v_rnd  = trunc(v) + 0.125;
	}
	else if (fracpart > 0.1875 && fracpart <= 0.3125)
	{
		v_rnd  = trunc(v) + 0.25;
	}
	else if (fracpart > 0.3125 && fracpart <= 0.4375)
	{
		v_rnd  = trunc(v) + 0.375;
	}
	else if (fracpart > 0.4375 && fracpart <= 0.5625)
	{
		v_rnd  = trunc(v) + 0.5;
	}
	else if (fracpart > 0.5625 && fracpart <= 0.6875)
	{
		v_rnd  = trunc(v) + 0.625;
	}
	else if (fracpart > 0.6875 && fracpart <= 0.8125)
	{
		v_rnd  = trunc(v) + 0.75;
	}
	else if (fracpart > 0.8125 && fracpart <= 0.9375)
	{
		v_rnd  = trunc(v) + 0.875;
	}
	else if (fracpart > 0.9375)
	{
		v_rnd  = trunc(v) + 1.0;
	}

	if (v_sign == -1)
		v_rnd = -v_rnd;

	return v_rnd;
}