#ifndef STAGEDATASET_H
#define STAGEDATASET_H

#include "exif.h"			// easy exif header
#include "AeroMap.h"		// application header
#include "Stage.h"			// base class

class StageDataset : Stage
{
public:

	virtual int Run() override;

private:

	struct ImageType
	{
		XString fileName;
		easyexif::EXIFInfo exif;
	};
	std::vector<ImageType> m_ImageList;

private:
	
	int WriteImageListText();
	int WriteImageListJson();

	int InitGeoref();
};

#endif // #ifndef STAGEDATASET_H
