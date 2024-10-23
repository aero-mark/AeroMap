#ifndef IMAGEPROC_H
#define IMAGEPROC_H

#include <opencv2/opencv.hpp>	// OpenCV

#include <QObject>

#include "MarkTypes.h"		// global type definitions
#include "XString.h"		// string class
#include "Calc.h"

class ImageProc : public QObject
{
	Q_OBJECT

public:

	// construction / destruction

	ImageProc();
	ImageProc(const char* fileName);
	virtual ~ImageProc();

	void Create(UInt16 width, UInt16 height);
	void CreateFrom(ImageProc* pSrcImage, int SrcImageIdx = 0, RectType* pRct = nullptr);

	// image access

	PixelType GetPixel(int imageIndex, UInt16 x, UInt16 y);	// no getR,getG, etc. too much overhead to recalc offset for components
	void SetPixel(int imageIndex, UInt16 x, UInt16 y, PixelType pix);
	const UInt8* GetData(unsigned int imageIndex = 0);		// return ptr to image's raw ARGB data

	// in-lines

	UInt32 GetHeight(int imageIndex = 0) { return (mp_Image[imageIndex] ? mp_Image[imageIndex]->Height : 0); }
	UInt32 GetWidth(int imageIndex = 0)  { return (mp_Image[imageIndex] ? mp_Image[imageIndex]->Width : 0); }

	bool IsValid(int x, int y) { return (x > -1 && x < mp_Image[0]->Width) && (y > -1 && y < mp_Image[0]->Height); }
	bool IsValid(PointType pt) { return (pt.x > -1 && pt.x < mp_Image[0]->Width) && (pt.y > -1 && pt.y < mp_Image[0]->Height); }
	bool IsValid(int imageIndex, int x, int y) { return (x > -1 && x < mp_Image[imageIndex]->Width) && (y > -1 && y < mp_Image[imageIndex]->Height); }

	XString GetFileName() { return ms_FileName; }

	// file io

	bool LoadFile(const char* fileName);
	bool SaveFile(const char* fileName);
	void SaveJpegFile(int imageIndex, const char* fileName, int quality = 75);
	void SaveBmpFile(int imageIndex, const char* fileName);
	void SavePngFile(int imageIndex, const char* fileName);

	// my operations

	bool AboveThreshold(int imageIndex, int x1, int y1, int x2, int y2, UInt8 threshold);
	UInt8 AverageIntensity(int imageIndex, int x1, int y1, int x2, int y2);
	void BlackBorder(int threshold = 150, int filter = 5);
	void BrightFilter(UInt8 bright);
	void CalcHistogram();
	void ContrastFilter(double denom);
	void ConvertToOpenGL();
	void Crop(RectType* pRect);
	void EdgeFilter(UInt8 threshold);
	void ExtractFilter(UInt8 red, UInt8 green, UInt8 blue, UInt8 threshold);
	void GrayScale();
	void IdentityFilter();
	void Replace();
	int  ReplaceColor(PixelType& pixSrc, PixelType& pixDst);
	void ResizeImage(double sx, double sy);
	void RotateImage(double angleRad, bool bInterpolate = false);		// my rotate function
	void RotateImageCV(double angleDeg);								// rotate using opencv
	void SharpnessFilter();

	// OpenCV operations

	// have same signature as opencv methods, without the first 2 (in/out) images;
	// these all implicitly use image[0] for input and image[1] for output

	void cvtColorCV(int code, int dstCn = 0);
	void GaussianBlurCV(cv::Size ksize, double sigmaX, double sigmaY = 0.0, int borderType = 4);
	void CannyCV(double threshold1, double threshold2, int apertureSize = 3, bool L2gradient = false);
	void erodeCV(cv::Size ksize, cv::Point anchor = cv::Point(-1, -1), int iterations = 1, int borderType = cv::BORDER_CONSTANT, const cv::Scalar& borderValue = cv::morphologyDefaultBorderValue());
	void dilateCV(cv::Size ksize, cv::Point anchor = cv::Point(-1, -1), int iterations = 1, int borderType = cv::BORDER_CONSTANT, const cv::Scalar& borderValue = cv::morphologyDefaultBorderValue());

	std::vector<std::vector<cv::Point>> findContoursCV(int mode, int method, cv::Point offset = cv::Point());

	// clipboard functions
	bool CopyToClipboard(int imageIndex);
	bool PasteFromClipboard();

private:

	struct ImageType		// image descriptor
	{
		int Height;				// height, pixels
		int Width;				// width, pixels
		UInt8* pData;			// image data 
	};

	ImageType* mp_Image[2];		// 0 = input image, 1 = output image

	struct HistType				// histogram output
	{
		UInt32 R[256];			// count of red values at [intensity]
		UInt32 G[256];
		UInt32 B[256];
		UInt32 Gray[256];
	};

	HistType m_Histogram;

	XString ms_FileName;		// graphics file name

private:

	ImageType* CreateImage(int width, int height);
	void RecreateImage(ImageType** ppImage, UInt16 width, UInt16 height);
	void DestroyImage(ImageType* pImage);
	void RecreateOutputIfNecessary();

	void LoadBmpFile(const char* fileName);
	void LoadJpegFile(const char* fileName);
	void LoadPngFile(const char* fileName);

	PixelType GetPixel(ImageType* pImage, UInt16 x, UInt16 y);
	void SetPixel(ImageType* pImage, UInt16 x, UInt16 y, PixelType pix);

	// convert internal image types to/from opencv mat type
	cv::Mat ImageToMat(int imageIndex);
	void    MatToImage(cv::Mat mat, int imageIndex = 0);

	// in-lines

	// calculate the byte offset into internal data buffer (forward or reverse) for pixel at (x,y)
	inline UInt32 CalcOffset(ImageType* pImage, int x, int y);
};

#endif // #ifndef IMAGEPROC_H
