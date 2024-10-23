// ImageProc.cpp
//
// Geo-specific image processing class.
//
// Notes:
//		- internally, there are 2 images - the "input" image and the "output" image;
//		  these are stored in mp_Image[0] and mp_Image[1] respectively
//		- the input image is not modified - transformations write to the output image
//		- the one exception is when the caller explicitly calls Replace(), copying 
//		  the output image to the input image
//
//		- all images are 32 bit, RGBA format; if you need a different format, just call
//		  GetXXX(), and a matching format will be created from the output image and made
//	      available
//

#include <QIODevice>
#include <QApplication>
#include <QClipboard>
#include <QImage>
#include <QMimedata>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <assert.h>
#include <io.h>

#include "BmpFile.h"		// interface to bitmap class
#include "JpegFile.h"		// interface to jpeg class
#include "PngFile.h"		// interface to png class
#include "ImageProc.h"

ImageProc::ImageProc()
{
	memset(&m_Histogram, 0, sizeof(m_Histogram));

	mp_Image[0] = nullptr;
	mp_Image[1] = nullptr;
}

ImageProc::ImageProc(const char* fileName)
	: ImageProc()
{
	// constructor that accepts graphics file name

	LoadFile(fileName);
}

ImageProc::~ImageProc()
{
	DestroyImage(mp_Image[0]);
	DestroyImage(mp_Image[1]);
}

bool ImageProc::LoadFile(const char* fileName)
{
	bool bReturn = false;

	XString sFileName = fileName;
	sFileName.Trim();

	// quick check for file existence before instantiating image file handlers
	int fd = _open(sFileName.c_str(), O_RDONLY | O_BINARY);
	if (fd > 0)
	{
		_close(fd);

		if (sFileName.EndsWithNoCase(".jpg") || sFileName.EndsWithNoCase(".jpeg"))
			LoadJpegFile(sFileName.c_str());
		else if (sFileName.EndsWithNoCase(".bmp"))
			LoadBmpFile(sFileName.c_str());
		else if (sFileName.EndsWithNoCase(".png"))
			LoadPngFile(sFileName.c_str());
		else
			assert(false);

		ms_FileName = sFileName;
		bReturn = true;
	}

	return bReturn;
}

bool ImageProc::SaveFile(const char* fileName)
{
	// Write the image file using the extension 
	// to guide the output format.
	//
	// This method will overwrite the file 
	// if it exists.
	//

	bool bReturn = false;

	XString sFileName = fileName;
	sFileName.Trim();

	if (sFileName.EndsWithNoCase(".jpg") || sFileName.EndsWithNoCase(".jpeg"))
		SaveJpegFile(1, sFileName.c_str());
	else if (sFileName.EndsWithNoCase(".bmp"))
		SaveBmpFile(1, sFileName.c_str());
	else if (sFileName.EndsWithNoCase(".png"))
		SavePngFile(1, sFileName.c_str());
	else
		assert(false);

	ms_FileName = sFileName;
	bReturn = true;

	return bReturn;
}

void ImageProc::LoadBmpFile(const char* fileName)
{
	// load bmp from disk - internally, all images regardless of source are stored
	// as 32-bit arrays
	//

	BmpFile* pBmp = new BmpFile();

	if (pBmp->Load(fileName))
	{
		// allocate our input & output images

		mp_Image[0] = CreateImage(pBmp->GetWidth(), pBmp->GetHeight());
		mp_Image[1] = CreateImage(pBmp->GetWidth(), pBmp->GetHeight());

		// populate our input/output images

		UInt8* pSrc = pBmp->GetData();
		int pixelCount = pBmp->GetHeight()*pBmp->GetWidth();

		UInt8* pDest1 = mp_Image[0]->pData;
		UInt8* pDest2 = mp_Image[1]->pData;

		for (int i = 0; i < pixelCount; ++i)
		{
			*pDest1++ = *pDest2++ = *pSrc++;	// src is 3 bytes
			*pDest1++ = *pDest2++ = *pSrc++;
			*pDest1++ = *pDest2++ = *pSrc++;
			*pDest1++ = *pDest2++ = 0;			// default alpha to 0
		}
	}

	delete pBmp;		// release it
}

void ImageProc::SaveBmpFile(int imageIndex, const char* fileName)
{
	// write bmp file
	//

	ImageType* pImage = mp_Image[imageIndex];
	BmpFile* pBmp = new BmpFile();		// type-specific io embedded in BMP class

	pBmp->Create(pImage->Width, pImage->Height);
	for (int x = 0; x < pImage->Width; ++x)
	{
		for (int y = 0; y < pImage->Height; ++y)
		{
			PixelType pix = GetPixel(imageIndex, x, y);
			pBmp->SetColor(x, y, pix.R, pix.G, pix.B);
		}
	}

	pBmp->Save(fileName);

	delete pBmp;
}

void ImageProc::LoadJpegFile(const char* fileName)
{
	UInt8* pBuf = nullptr;			// pixel data buffer
	unsigned int Width = 0;
	unsigned int Height = 0;

	// read to buffer tmp
	pBuf = JpegFile::JpegFileToRGB(fileName, &Width, &Height);
	if (!pBuf)
	{
		Logger::Write(__FUNCTION__, "JpegFile::JpegFileToRGB() failed");
		return;
	}

	// allocate our input & output images

	mp_Image[0] = CreateImage(Width, Height);
	mp_Image[1] = CreateImage(Width, Height);

	// now, copy it to internal pixel structure while mapping from 24 to 32 bit
	// pixel format

	UInt8* pSrc = pBuf;
	UInt8* pDest1 = mp_Image[0]->pData;
	UInt8* pDest2 = mp_Image[1]->pData;

	for (unsigned int y = 0; y < Height; ++y)
	{
		for (unsigned int x = 0; x < Width; ++x)
		{
			*pDest1++ = *pDest2++ = *pSrc++;
			*pDest1++ = *pDest2++ = *pSrc++;
			*pDest1++ = *pDest2++ = *pSrc++;
			*pDest1++ = *pDest2++ = 0;
		}
	}

	delete pBuf;
}

void ImageProc::SaveJpegFile(int imageIndex, const char* fileName, int quality /* = 75 */)
{
	// write Jpeg file
	//

	ImageType* pImage = mp_Image[imageIndex];

	// copy to 24-bit buffer

	UInt8* pBuf = new UInt8[pImage->Width*pImage->Height * 3];

	UInt8* pDest = pBuf;
	UInt8* pSrc = pImage->pData;
	for (int y = 0; y < pImage->Height; ++y)
	{
		for (int x = 0; x < pImage->Width; ++x)
		{
			*pDest++ = *pSrc++;
			*pDest++ = *pSrc++;
			*pDest++ = *pSrc++;
			pSrc++;
		}
	}

	// write it
	bool bStatus = JpegFile::RGBToJpegFile(	fileName,
											pBuf,
											pImage->Width,
											pImage->Height,
											true,				// true = color, false = grayscale
											quality);			// 0..100
	if (!bStatus)
	{
		Logger::Write(__FUNCTION__, "Error writing Jpeg file '%s'.", fileName);
	}

	delete pBuf;
}

void ImageProc::LoadPngFile(const char* fileName)
{
	// load PNG from disk - internally, all images regardless of source are stored
	// as 32-bit arrays
	//

	PngFile* pPng = new PngFile(fileName);

	if (pPng->GetWidth() > 0)
	{
		// allocate our input & output images

		mp_Image[0] = CreateImage(pPng->GetWidth(), pPng->GetHeight());
		mp_Image[1] = CreateImage(pPng->GetWidth(), pPng->GetHeight());

		// populate our input/output images

		for (UInt32 x = 0; x < pPng->GetWidth(); ++x)
		{
			for (UInt32 y = 0; y < pPng->GetHeight(); ++y)
			{
				PixelType pix = pPng->GetPixel(x, y);
				SetPixel(0, x, y, pix);
			}
		}
//TODO:
//we don't yet have a GetData()
		//UInt8* pSrc = pPng->GetData();
		//int pixelCount = pPng->GetHeight()*pPng->GetWidth();

		//UInt8* pDest1 = mp_Image[0]->pData;
		//UInt8* pDest2 = mp_Image[1]->pData;

		//for (int i = 0; i < pixelCount; ++i)
		//{
		//	*pDest1++ = *pDest2++ = *pSrc++;	// src is 3 bytes
		//	*pDest1++ = *pDest2++ = *pSrc++;
		//	*pDest1++ = *pDest2++ = *pSrc++;
		//	*pDest1++ = *pDest2++ = 0;			// default alpha to 0
		//}
	}

	delete pPng;		// release it

	IdentityFilter();
}

void ImageProc::SavePngFile(int imageIndex, const char* fileName)
{
	// write PNG file
	//

	ImageType* pImage = mp_Image[imageIndex];

	PngFile* pPngFile = new PngFile();

	pPngFile->Create(pImage->Width, pImage->Height, 3);
	for (int x = 0; x < pImage->Width; ++x)
	{
		for (int y = 0; y < pImage->Height; ++y)
		{
			PixelType pix = GetPixel(imageIndex, x, y);
			pPngFile->SetPixel(x, y, pix);
		}
	}

	// write it
	int status = pPngFile->Save(fileName);

	delete pPngFile;

	if (status != 0)
	{
		Logger::Write(__FUNCTION__, "Error writing PNG file '%s'.", fileName);
	}
}

void ImageProc::RecreateOutputIfNecessary()
{
	// recreate output image if it doesn't match the format of the input image

	if (mp_Image[0]->Width != mp_Image[1]->Width || mp_Image[0]->Height != mp_Image[1]->Height)
	{
		RecreateImage(&mp_Image[1], mp_Image[0]->Width, mp_Image[0]->Height);
	}
}

void ImageProc::RecreateImage(ImageType** ppImage, UInt16 width, UInt16 height)
{
	//	free and reallocate an image descriptor
	//
	//	inputs:
	//		pImage = address of ptr to image to be replaced
	//		Width = width of image, pixels
	//		Height = height of image, pixels
	//
	//	outputs:
	//		pImage = address of new image descriptor
	//

	DestroyImage( *ppImage );					// release existing image
	*ppImage = CreateImage( width, height );	// create new image
}

void ImageProc::Create(UInt16 width, UInt16 height)
{
	//	create a new image
	//
	//    inputs:
	//        width = width of image, pixels
	//		height = height of image, pixels
	//

	// recreate our source & dest images with the new size
	RecreateImage(&mp_Image[0], width, height);
	RecreateImage(&mp_Image[1], width, height);

	// blank image

	UInt32* pDest = (UInt32*)mp_Image[0]->pData;
	for (UInt32 pixelIdx = 0; pixelIdx < (UInt32)(width*height); ++pixelIdx)
	{
		*pDest++ = 0;
	}

	IdentityFilter();	// blit input image to output image
}

ImageProc::ImageType* ImageProc::CreateImage(int Width, int Height)
{
	//	allocate an image descriptor
	//
	//	inputs:
	//		Width = width of image, pixels
	//		Height = height of image, pixels
	//

	ImageType* pReturn = new ImageType();

	pReturn->Width = Width;
	pReturn->Height = Height;
	pReturn->pData = new UInt8[Width * Height * 4];

	return pReturn;
}

void ImageProc::CreateFrom(ImageProc* pSrcImage, int SrcImageIdx /* = 0 */, RectType* pRct /* = nullptr */)
{
	// create a new source image from another Image object

	// no dest image index is accepted because it only makes sense to write TO the input
	// image, the output image is recreated implicitly

	// inputs:
	//      pSrcImage	= ptr to source Image object
	//		SrcImageIdx	= image index of source image
	//		pRct		= bounding rectange of area to be read, if nullptr entire image copied
	//

	if (!pSrcImage)
		return;
	if (SrcImageIdx < 0 || SrcImageIdx > 1)
		return;

	int w = pSrcImage->GetWidth(SrcImageIdx);
	int h = pSrcImage->GetHeight(SrcImageIdx);
	if (pRct)
	{
		w = pRct->x1 - pRct->x0 + 1;
		h = pRct->y1 - pRct->y0 + 1;
	}

	RecreateImage(&mp_Image[0], w, h);
	RecreateImage(&mp_Image[1], w, h);

	// since "friend", can just do blit directly from source data bits

	UInt32* pDest = (UInt32*)mp_Image[0]->pData;			// destination is byte 0 of data area
	for (int y = pRct->y0; y <= pRct->y1; ++y)
	{
		// calc offset to start of current scan line in source image
		UInt32* pSrc = (UInt32*)pSrcImage->mp_Image[SrcImageIdx]->pData
			+ (y*pSrcImage->GetWidth(SrcImageIdx) + pRct->x0);

		for (int x = pRct->x0; x <= pRct->x1; ++x)
			*pDest++ = *pSrc++;
	}

	IdentityFilter();	// blit input image to output image
}

void ImageProc::DestroyImage(ImageType* pImage)
{
	// free an image descriptor
	//

	if (!pImage)
		return;

	delete pImage->pData;		// delete image data
	delete pImage;				// delete descriptor
}

//void ImageProc::GetImageRGB()
///*
//	create a separate RGB formatted image and return ptr
////TODO:
////likewise for other formats
//*/
//{
//	if (m_ImageFormat == format)		// already in the desired format
//		return;
//
//	int pixelCount = mp_Image[0]->Width*mp_Image[0]->Height;
//
//	// create an output image that we will write to, then blit it back to input - at the 
//	// end we will have 2 matching surfaces with not too much excessive copying
//	RecreateImage( &mp_Image[1], mp_Image[0]->Width, mp_Image[0]->Height, format );
//
//	UInt8* pSrc = mp_Image[0]->pData;
//	UInt8* pDest = mp_Image[1]->pData;
//
//	for (int i = 0; i < pixelCount; i++)
//	{
//		if (m_ImageFormat == FORMAT_ARGB)		// source = ARGB
//		{
//			if (format == FORMAT_RGBA)			// dest = RGBA
//			{
//				*pDest++ = pSrc[3];
//				*pDest++ = pSrc[0];
//				*pDest++ = pSrc[1];
//				*pDest++ = pSrc[2];
//			}
//			else if (format == FORMAT_RGB)		// dest = RGB
//			{
//				*pDest++ = pSrc[0];
//				*pDest++ = pSrc[1];
//				*pDest++ = pSrc[2];
//			}
//			pSrc += 4;
//		}
//	}
//
////TODO:
////recreate image[0]
////blit image[1] to image[0]
//
//	m_ImageFormat = format;
//}

void ImageProc::SetPixel(int imageIndex, UInt16 x, UInt16 y, PixelType pix)
{
	// set a single pixel
	// 
	// inputs:
	//		imageIndex 	= 0 => input image
	//					= 1 => output image
	//		x,y      	= coordinates of pixel
	//		pix		   	= 32 bit pixel color

	if (imageIndex < 0 || imageIndex > 1)
		return;

	SetPixel(mp_Image[imageIndex], x, y, pix);
}

void ImageProc::SetPixel(ImageType* pImage, UInt16 x, UInt16 y, PixelType pix)
{
	// set a single pixel, overload that accepts image ptr
	//
	// inputs:
	// 		imageIndex = 0 => input image
	// 				     1 => output image
	// 		x,y	= coordinates of pixel
	// 		pix	= 32 bit pixel color

	if (!pImage)
		return;

	if (x > pImage->Width - 1 || y > pImage->Height - 1)
		return;

	// calculate pixel offset
	UInt32 offset = CalcOffset(pImage, x, y);

	pImage->pData[offset] = pix.R;
	pImage->pData[offset + 1] = pix.G;
	pImage->pData[offset + 2] = pix.B;
	pImage->pData[offset + 3] = pix.A;
}

PixelType ImageProc::GetPixel(int imageIndex, UInt16 x, UInt16 y)
{
	//	get a single pixel
	//
	//	inputs:
	//		imageIndex = 0 => input image
	//				     1 => output image
	//		x,y = coordinates of pixel
	//
	//	outputs:
	//		return = 32 bit pixel color
	//

	PixelType pix;

	if (imageIndex < 0 || imageIndex > 1)
		return pix;

	pix = GetPixel(mp_Image[imageIndex], x, y);

	return pix;
}

PixelType ImageProc::GetPixel(ImageType* pImage, UInt16 x, UInt16 y)
{
	// get a single pixel, overload that accepts an image
	//
	// inputs:
	//		pImage = image descriptor
	//		x,y    = coordinates of pixel
	//
	// output:
	//		return = 32 bit pixel color

	PixelType pix;

	if (!pImage)
		return pix;

	if (x > pImage->Width - 1 || y > pImage->Height - 1)
		return pix;

	// calculate pixel offset
	UInt32 offset = CalcOffset(pImage, x, y);

	pix.R = pImage->pData[offset];
	pix.G = pImage->pData[offset + 1];
	pix.B = pImage->pData[offset + 2];
	pix.A = pImage->pData[offset + 3];

	return pix;
}

const UInt8* ImageProc::GetData(unsigned int imageIndex)
{
	// return ptr to image's raw ARGB data

	if (imageIndex > 1)
		return nullptr;

	ImageType* pImage = mp_Image[imageIndex];
	return pImage->pData;
}

UInt32 ImageProc::CalcOffset(ImageType* pImage, int x, int y)
{
	return (y * pImage->Width + x) * 4;
}

//==========================================================================================================================
// image transformations 

void ImageProc::ConvertToOpenGL()
{
	// convert image to OpenGL texture compatible RGBA format

	// "converting" is nothing more that packing the pixel data (image file formats
	// often have padding in them) and expanding to RGBA

	RecreateOutputIfNecessary();

	for (unsigned int y = 0; y < GetHeight(); ++y)
	{
		for (unsigned x = 0; x < GetWidth(); ++x)
		{
			// get rgb values of pixel at (x,y)
			PixelType pix = GetPixel( 0, x, y );
			SetPixel( 1, x, y, pix );
		}
	}
}

void ImageProc::BlackBorder(int threshold, int filter)
{
	// set darker image border to solid black
	//
	// filters out runs of "light" pixels smaller than "filter"
	//
	// inputs:
	//		threshold = simple intensity at wich we are "on stamp"
	//		filter = filter out blobs smaller than this (in scan dir only)

	// For this operation, always reset output
	IdentityFilter();

	// process top/bottom

	for (UInt32 x = 0; x < GetWidth(); ++x)
	{
		// top

		int lightCtr = 0;        // current # of contiguous light pixels
		UInt32 edge = 0;         // pixel offset of light edge

		for (UInt32 y = 0; y < GetHeight(); ++y)
		{
			PixelType pix = GetPixel(0, x, y);
			if (pix.GetGray() > threshold)
			{
				// filter contiguous light pixels
				if (++lightCtr > filter)
					break;
			}
			else
			{
				// reset counter
				lightCtr = 0;
				edge = y;
			}
		}
		for (UInt32 y = 0; y < edge; ++y)
		{
			SetPixel(1, x, y, PixelType(0, 0, 0, 0xFF));
		}

		// bottom

		lightCtr = 0;
		edge = GetHeight() - 1;

		for (UInt32 y = GetHeight() - 1; y > 0; --y)
		{
			PixelType pix = GetPixel(0, x, y);
			if (pix.GetGray() > threshold)
			{
				// filter contiguous light pixels
				if (++lightCtr > filter)
					break;
			}
			else
			{
				// reset counter
				lightCtr = 0;
				edge = y;
			}
		}
		for (UInt32 y = GetHeight() - 1; y > edge; --y)
		{
			SetPixel(1, x, y, PixelType(0, 0, 0, 0xFF));
		}
	}
	 
	// process left/right

	for (UInt32 y = 0; y < GetHeight(); ++y)
	{
		// left 

		int lightCtr = 0;        // current # of contiguous light pixels
		UInt32 edge = 0;         // pixel offset of light edge

		for (UInt32 x = 0; x < GetWidth(); ++x)
		{
			PixelType pix = GetPixel(0, x, y);

			if (pix.GetGray() > threshold)
			{
				if (++lightCtr > filter)
					break;
			}
			else
			{
				lightCtr = 0;
				edge = x;
			}
			for (UInt32 x = 0; x < edge; ++x)
			{
				SetPixel(1, x, y, PixelType(0, 0, 0, 0xFF));
			}
		}

		// right 

		lightCtr = 0;
		edge = GetHeight() - 1;

		for (UInt32 x = GetWidth() - 1; x > 0; --x)
		{
			PixelType pix = GetPixel(0, x, y);

			if (pix.GetGray() > threshold)
			{
				if (++lightCtr > filter)
					break;
			}
			else
			{
				lightCtr = 0;
				edge = x;
			}
			for (UInt32 x = GetWidth() - 1; x > edge; --x)
			{
				SetPixel(1, x, y, PixelType(0, 0, 0, 0xFF));
			}
		}
	}
}

void ImageProc::GrayScale()
{
	//	convert color image to grayscale
	//
	//	see "ImageProc.pdf" for description of algorithm
	//

	RecreateOutputIfNecessary();

	for (UInt32 x = 0; x < GetWidth(); ++x)
	{
		for (UInt32 y = 0; y < GetHeight(); ++y)
		{
			// get rgb values of pixel at (x,y)

			PixelType pix = GetPixel(0, x, y);

			// perform processing - we're actually projecting the color vector onto the "gray vector",
			// dir = (1,1,1), but by the time you normalize it and rearrange the expressions, it simplifies
			// to the simple average of the rgb values

			UInt8 Gray = (pix.R + pix.G + pix.B) / 3;

			// set output color value

			SetPixel(1, x, y, PixelType(Gray, Gray, Gray, 0xFF));
		}
	}
}

bool ImageProc::AboveThreshold(int imageIndex, int x1, int y1, int x2, int y2, UInt8 threshold)
{
	// return true if ALL pixels in region are above a specified color threshold
	// 
	// inputs:
	//		image index		= 0/1 for source/dest
	// 		x1,y1,x2,y2		= region to test, pixel coordinates
	// 		threshold		= color threshold, per channel
	// 
	// outputs:
	//		return = true => all pixels in specified region have grayscale color intensity
	// 						 above specified threshold

	bool bReturn = true;

	if (x1 > x2)
		swap(x1, x2);
	if (y1 > y2)
		swap(y1, y2);

	// check region for valid extents; return false because - even though we could take intersection of the
	// 2 regions and still probably come up with a valid region - it's not what caller most likely wants;
	// the test here is "Do all N of the pixels in my dX x dY test region satisfy my criteria?" Since we
	// no longer have N pixels to test, returning true doesn't make any sense;

	if (x1 < 0 || y1 < 0 || x2 > mp_Image[imageIndex]->Width - 1 || y2 > mp_Image[imageIndex]->Height - 1)
		return false;

	for (int x = x1; x <= x2; ++x)
	{
		for (int y = y1; y <= y2; ++y)
		{
			PixelType color = GetPixel(imageIndex, x, y);

			// we're actually projecting the color vector onto the "gray vector",
			// dir = (1,1,1), but by the time you normalize it and rearrange the expressions, it simplifies
			// to the simple average of the rgb values

			// get magnitude of color vector
			UInt8 Gray = (color.R + color.G + color.B) / 3;

			if (Gray < threshold)		// below threshold, region does not pass test
			{
				bReturn = false;
				x = x2; y = y2;
			}
		}
	}

	return bReturn;
}

UInt8 ImageProc::AverageIntensity(int imageIndex, int x1, int y1, int x2, int y2)
{
	//	return the average intensity of the pixels in the specified region
	//

	UInt32 nSumR = 0;
	UInt32 nSumG = 0;
	UInt32 nSumB = 0;

	if (x2 < x1)
		swap(x2, x1);
	if (y2 < y1)
		swap(y2, y1);

	for (int x = x1; x <= x2; ++x)
	{
		for (int y = y1; y <= y2; ++y)
		{
			PixelType pix = GetPixel(imageIndex, x, y);
			nSumR += pix.R;
			nSumG += pix.G;
			nSumB += pix.B;
		}
	}

	// as with the grayscale filters, the vector math is reduced to nothing leaving the
	// final result as a simple average

	UInt32 nGray = (nSumR + nSumG + nSumB) / (3 * (x2 - x1 + 1) * (y2 - y1 + 1));

	return (UInt8)nGray;
}

void ImageProc::EdgeFilter(UInt8 threshold)
{
	// edge detection filter - c version
	//
	// see "ImageProc.pdf" for description of algorithm
	//

	float fThreshold = (float)threshold;		// cast to compare to 3d distances

	RecreateOutputIfNecessary();

	for (int x = 0; x < mp_Image[0]->Width - 1; ++x)
	{
		for (int y = 0; y < mp_Image[0]->Height - 1; ++y)
		{
			// get rgb values of "this" pixel

			PixelType pix = GetPixel(0, x, y);
			PixelType pixe = GetPixel(0, x + 1, y);		// get rgb values of right pixel
			PixelType pixs = GetPixel(0, x, y + 1);		// get rgb values of below pixel

			// perform processing

			VEC3 vColor(pix.R, pix.G, pix.B);			// get input colors in vector form
			VEC3 vColor1(pixe.R, pixe.G, pixe.B);
			VEC3 vColor2(pixs.R, pixs.G, pixs.B);

			// get distance between this color vector and neighboring color vectors
			//TODO:
			// this isn't the angular distance between
			// the 2 vectors, it's just the distance between the 3 color "points" in 3d
			// space - would be interesting use angle between vectors & see how results
			// compare

			float delta1 = (float)Distance(vColor, vColor1);
			float delta2 = (float)Distance(vColor, vColor2);

			// set output color value

			// if color vectors farther apart than threshold
			if (delta1 > fThreshold || delta2 > fThreshold)
				SetPixel(1, x, y, PixelType(0xFF, 0xFF, 0xFF, 0xFF));
			else
				SetPixel(1, x, y, PixelType(0x00, 0x00, 0x00, 0x00));
		}
	}
}

void ImageProc::ContrastFilter(double denom)
{
	// increase/decrease color contrast in source image
	//
	// see "ImageProc.pdf" for description of algorithm
	//

	RecreateOutputIfNecessary();

	// build contrast transform table

	UInt8 Contrast_Transform[256];

	double contrast = PI / denom;	// at PI/4, curve is diagonal from (0,0) to (0,255) so input == output

	for (int i = 0; i < 256; ++i)
	{
		double f1 = 128.0 - 128.0*tan(contrast);		// low cutoff
		double f2 = 128.0 + 128.0*tan(contrast);		// high cutoff

		if (i < f1)							// off bottom - map to minimum intensity
			Contrast_Transform[i] = 0;
		else if (i > f2)					// off top - map to maximum intensity
			Contrast_Transform[i] = 255;
		else								// in range - use value from intensity curve
			Contrast_Transform[i] = (UInt8)((i - 128) / tan(contrast) + 128);
	}

	// apply contrast transform

	for (int x = 0; x < mp_Image[0]->Width; ++x)
	{
		for (int y = 0; y < mp_Image[0]->Height; ++y)
		{
			// get rgb values of pixel at (x,y)

			PixelType pix = GetPixel(0, x, y);

			// use the rgb values as indices into transform table

			UInt8 R2 = Contrast_Transform[pix.R];
			UInt8 G2 = Contrast_Transform[pix.G];
			UInt8 B2 = Contrast_Transform[pix.B];

			// set output color value

			SetPixel(1, x, y, PixelType(R2, G2, B2, 0xFF));
		}
	}
}

void ImageProc::ExtractFilter(UInt8 red, UInt8 green, UInt8 blue, UInt8 threshold)
{
	//	color extraction filter
	//
	//	see "ImageProc.pdf" for description of algorithm
	//

	RecreateOutputIfNecessary();

	VEC3 vTargetColor = VEC3(red, green, blue);
	for (int x = 0; x < mp_Image[0]->Width; ++x)
	{
		for (int y = 0; y < mp_Image[0]->Height; ++y)
		{
			// get rgb values of "this" pixel

			PixelType pix = GetPixel(0, x, y);

			VEC3 vColor(pix.R, pix.G, pix.B);		// get input colors in vector form

			// get "difference" between 2 colors
			double delta = Distance(vTargetColor, vColor);

			// set output color value

			// if sample color "close" to target color
			if (delta < (double)threshold)
				SetPixel(1, x, y, PixelType(pix.R, pix.G, pix.B, 0xFF));
			else
				SetPixel(1, x, y, PixelType(0xFF, 0xFF, 0xFF, 0xFF));
		}
	}
}

void ImageProc::BrightFilter(UInt8 bright)
{
	//	increase/decrease color contrast in source image
	//
	//	as implemented, algorithm is pretty trivial - just add scalar value to each color - but if combined
	//	with color histogram could be used to dramatically clear up an image; for example, if image was very
	//	dark with 1 light spot in the middle, increasing brightness of entire image would saturate bright spot
	//	by the time rest of image became clear; so using the frequency distribution, add more the dark areas
	//	and less (even subtract) from the bright areas to clear up the overall image;
	//
	//	see "ImageProc.pdf" for description of algorithm
	//
	// inputs:
	//		bright = value to add to each pixel/channel

	RecreateOutputIfNecessary();

	UInt8 Brightness_Transform[256];

	// build transform table

	for (int i = 0; i < 256; ++i)
	{
		if (i + bright > 255)			// off top - map to maximum intensity
			Brightness_Transform[i] = 255;
		else								// in range - use value from curve
			Brightness_Transform[i] = i + bright;
	}

	// apply transform

	for (int x = 0; x < mp_Image[0]->Width; ++x)
	{
		for (int y = 0; y < mp_Image[0]->Height; ++y)
		{
			// get rgb values of pixel at (x,y)

			PixelType pix1 = GetPixel(0, x, y);

			UInt8 R2 = Brightness_Transform[pix1.R];
			UInt8 G2 = Brightness_Transform[pix1.G];
			UInt8 B2 = Brightness_Transform[pix1.B];

			// set output color value

			SetPixel(1, x, y, PixelType(R2, G2, B2, 0xFF));
		}
	}
}

void ImageProc::ResizeImage(double sx, double sy)
{
	//	see "ImageProc.pdf" for description of algorithm
	//

	int outSizeX = (int)(sx * mp_Image[0]->Width);
	int outSizeY = (int)(sy * mp_Image[0]->Height);

	RecreateImage(&mp_Image[1], outSizeX, outSizeY);	// create the bitmap that will be used for output

	// scale horizontally

	int x, y;
	for (y = 0; y < mp_Image[0]->Height; ++y)
	{
		for (x = 0; x < outSizeX - 1; ++x)
		{
			double srcX = (double)x / sx;			// calc floating point source column
			double deltaX = fmod(srcX, 1.0);	// get fractional part

			// get colors at x and x+1

			PixelType pix1 = GetPixel(0, (int)(srcX), y);
			PixelType pix2 = GetPixel(0, (int)(srcX + 1), y);

			// interpolate them

			PixelType pix = pix1.Lerp(deltaX, pix2);

			// set output color value

			SetPixel(1, x, y, PixelType(pix.R, pix.G, pix.B, 0xFF));
		}
	}

	// move to input
	Replace();

	// scale vertically

	for (y = 0; y < outSizeY - 1; ++y)
	{
		double srcY = (double)y / sy;			// calc floating point source column
		double deltaY = fmod(srcY, 1.0);		// get fractional part

		for (x = 0; x < outSizeX; ++x)
		{
			// get colors at y and y+1

			PixelType pix1 = GetPixel(0, x, (int)(srcY));
			PixelType pix2 = GetPixel(0, x, (int)(srcY + 1));

			// interpolate them

			PixelType pix = pix1.Lerp(deltaY, pix2);

			// set output color value

			SetPixel(1, x, y, PixelType(pix.R, pix.G, pix.B, 0xFF));
		}
	}

	// move to input
	Replace();
}

void ImageProc::RotateImageCV(double angleDeg)
{
	// Rotate image using OpenCV

	RecreateOutputIfNecessary();

	double cx = mp_Image[0]->Width / 2;		// center of rotation
	double cy = mp_Image[0]->Height / 2;
	cv::Point2f center(cx, cy);

	cv::Mat in;
	cv::Mat out;
	cv::Size2f size(mp_Image[0]->Width, mp_Image[0]->Height);

	// use image 0 as input
	in = ImageToMat(0);

	cv::Mat matRotate = cv::getRotationMatrix2D(center, angleDeg, 1.0);
	cv::warpAffine(in, out, matRotate, size);

	// move image with contours drawn to output
	MatToImage(out, 1);
}

void ImageProc::RotateImage(double angle, bool bInterpolate /* = false */)
{
	//	rotate the bitmap
	//
	//	key here is to drive off destination rather than source - for each pixel in output image,
	//	get the corresponding color from the input image; that way there are no holes in result;
	//
	//	see "ImageProc.pdf" for description of algorithm
	//
	//	inputs:
	//		angle = rotation angle, radians
	//		bInterpolate = true => interpolate color values of destination pixels
	//
	//

	RecreateOutputIfNecessary();

	double cx = mp_Image[0]->Width / 2;		// center of rotation
	double cy = mp_Image[0]->Height / 2;

	for (int x = 0; x < mp_Image[0]->Width; ++x)
	{
		for (int y = 0; y < mp_Image[0]->Height; ++y)
		{
			// get source color

			double x1, y1;
			Rotate2d(angle, (double)x, (double)y, &x1, &y1, cx, cy);

			// get colors at source pixel

			int srcX = (int)x1;
			int srcY = (int)y1;

			double dx = fmod(x1, 1.0);
			double dy = fmod(y1, 1.0);

			// if not rotated off surface
			if (srcX > -1 && srcX < mp_Image[0]->Width - 1 && srcY > -1 && srcY < mp_Image[0]->Height - 1)
			{
				PixelType pix = GetPixel(0, srcX, srcY);

				if (bInterpolate)
				{
					// interp color between 4 surrounding points in source map - gives output image that looks
					// much "nicer" but (barely noticeable) blur due to averaging

					PixelType pixTop = GetPixel(0, srcX, srcY).Lerp(dx, GetPixel(0, srcX + 1, srcY));
					PixelType pixBottom = GetPixel(0, srcX, srcY + 1).Lerp(dx, GetPixel(0, srcX + 1, srcY + 1));
					pix = pixTop.Lerp(dy, pixBottom);
				}

				// set output color value
				SetPixel(1, x, y, PixelType(pix.R, pix.G, pix.B, 0xFF));
			}
		}
	}
}

void ImageProc::SharpnessFilter()
{
	// from ImageProc.pdf
	//
	// at least got it to run and produce a recognizable image, but doesn't look as good as the
	// example - still have errors

	const int SHARP_W = 3;
	const int SHARP_H = 3;

	Int32 sumr = 0;
	Int32 sumg = 0;
	Int32 sumb = 0;
	Int32 sharpen_filter[SHARP_W][SHARP_H] =
	{
		{  0, -1,  0 },
		{ -1,  5, -1 },
		{  0, -1,  0 }
	};

	Int32 sharp_sum = 1;		// looks like sum of matrix values

	for (int i = 1; i < mp_Image[0]->Width - 1; ++i)
	{
		for (int j = 1; j < mp_Image[0]->Height - 1; ++j)
		{
			sumr = 0;
			sumg = 0;
			sumb = 0;
			for (int k = 0; k < SHARP_W; ++k)
			{
				for (int l = 0; l < SHARP_H; ++l)
				{
					int x = i - ((SHARP_W - 1) >> 1) + k;	// source pixel coordinates
					int y = j - ((SHARP_H - 1) >> 1) + l;

					PixelType pix = GetPixel(0, x, y);

					sumr += pix.R*sharpen_filter[k][l];
					sumg += pix.G*sharpen_filter[k][l];
					sumb += pix.B*sharpen_filter[k][l];
				}	// for l
			}		// for k

			sumr /= sharp_sum;
			sumb /= sharp_sum;
			sumg /= sharp_sum;

			//should i be doing this, or is presence of out-of-range result indication of error?
			if (sumr < 0) sumr = 0;
			if (sumg < 0) sumg = 0;
			if (sumb < 0) sumb = 0;
			if (sumr > 255) sumr = 255;
			if (sumg > 255) sumg = 255;
			if (sumb > 255) sumb = 255;

			SetPixel(1, i, j, PixelType((UInt8)sumr, (UInt8)sumg, (UInt8)sumb, 0xFF));
		}		// for j
	}			// for i
}

void ImageProc::CalcHistogram()
{
	// build a color histogram
	//
	// using the input image, counts # of each color (gray and rgb) at each intensity
	//
	// see "ImageProc.pdf" for description of algorithm

	memset(&m_Histogram, 0, sizeof(m_Histogram));

	// count the occurrences of each intensity

	//long nMaxCount = 0;
	for (int x = 0; x < mp_Image[0]->Width; ++x)
	{
		for (int y = 0; y < mp_Image[0]->Height; ++y)
		{
			// get rgb values of pixel at (x,y)

			PixelType pix = GetPixel(0, x, y);

			m_Histogram.R[pix.R]++;
			m_Histogram.G[pix.G]++;
			m_Histogram.B[pix.B]++;

			UInt8 gray = (pix.R + pix.G + pix.B) / 3;
			m_Histogram.Gray[gray]++;
		}
	}
}

void ImageProc::IdentityFilter()
{
	// apply null transform to input image creating output image
	// that is identical to input
	//

	RecreateOutputIfNecessary();

	int pixelCount = GetWidth()*GetHeight();

	UInt8* pSrc = mp_Image[0]->pData;
	UInt8* pDst = mp_Image[1]->pData;
	for (int i = 0; i < pixelCount; ++i)
	{
		*pDst++ = *pSrc++;
		*pDst++ = *pSrc++;
		*pDst++ = *pSrc++;
		*pDst++ = *pSrc++;
	}
}

void ImageProc::Replace()
{
	// replace source input image with output image
	//

	// ensure source is same size as dest (for this op, 0 is dest)
	if (mp_Image[0]->Width != mp_Image[1]->Width || mp_Image[0]->Height != mp_Image[1]->Height)
	{
		// recreate the source, not the dest
		RecreateImage(&mp_Image[0], mp_Image[1]->Width, mp_Image[1]->Height);
	}

	for (int x = 0; x < mp_Image[0]->Width; ++x)
	{
		for (int y = 0; y < mp_Image[0]->Height; ++y)
		{
			SetPixel(0, x, y, GetPixel(1, x, y));
		}
	}
}

int ImageProc::ReplaceColor(PixelType& pixSrc, PixelType& pixDst)
{
	// replace one color with another
	//
	// inputs:
	//		pixSrc = color to be replaced
	//		pixDst = new color
	// outputs:
	//		return = # of pixels modified
	//

	int pixelCount = 0;

	for (int x = 0; x < mp_Image[0]->Width; ++x)
	{
		for (int y = 0; y < mp_Image[0]->Height; ++y)
		{
			PixelType pix = GetPixel(0, x, y);
			if (pix.CompareRGB(pixSrc))
			{
				SetPixel(1, x, y, pixDst);
				++pixelCount;
			}
		}
	}

	return pixelCount;
}

void ImageProc::Crop(RectType* pRect)
{
	// crop image
	//
	// inputs:
	//		pRect = region within current image to be cropped
	//

	// validate region

	if (pRect->x0 > mp_Image[0]->Width)
		return;
	if (pRect->y0 > mp_Image[0]->Height)
		return;

	if (pRect->x1 > mp_Image[0]->Width - 1)
		pRect->x1 = mp_Image[0]->Width - 1;
	if (pRect->y1 > mp_Image[0]->Height - 1)
		pRect->y1 = mp_Image[0]->Height - 1;

	int w = pRect->x1 - pRect->x0 + 1;
	int h = pRect->y1 - pRect->y0 + 1;

	RecreateImage(&mp_Image[1], w, h);	// create new output image

	// slow blit
	for (int x = pRect->x0; x <= pRect->x1; ++x)
	{
		for (int y = pRect->y0; y <= pRect->y1; ++y)
		{
			PixelType pix = GetPixel(0, x, y);
			SetPixel(1, (UInt16)(x-pRect->x0), (UInt16)(y-pRect->y0), pix);
		}
	}

	Replace();		// replace source image with dest image
}

//========================================================================================================================
// opencv methods
//

void ImageProc::cvtColorCV(int code, int dstCn)
{
	// convert color space using OpenCV cvtColor()
	// method.
	//

	cv::Mat in;
	cv::Mat out;

	// use image 0 as input
	in = ImageToMat(0);

	//cv::namedWindow("image", cv::WINDOW_NORMAL);
	//cv::imshow("image", in);
	//cv::waitKey(0);

	cv::cvtColor(in, out, code, dstCn);

	//cv::imshow("image", out);
	//cv::waitKey(0);

	// place output in image 1
	MatToImage(out, 1);
}

void ImageProc::GaussianBlurCV(cv::Size ksize, double sigmaX, double sigmaY, int borderType)
{
	// Apply OpenCV GaussianBlur() method to image.
	//

	cv::Mat in;
	cv::Mat out;

	// use image 0 as input
	in = ImageToMat(0);

	//cv::namedWindow("image", cv::WINDOW_NORMAL);
	//cv::imshow("image", in);
	//cv::waitKey(0);

	cv::GaussianBlur(in, out, ksize, sigmaX, sigmaY, borderType);

	//cv::imshow("image", out);
	//cv::waitKey(0);

	// place output in image 1
	MatToImage(out, 1);
}

void ImageProc::CannyCV(double threshold1, double threshold2, int apertureSize, bool L2gradient)
{
	// void Canny(InputArray image, OutputArray edges, double threshold1, double threshold2, int apertureSize = 3, bool L2gradient = false)
	//		image – single-channel 8-bit input image
	//		edges – output edge map; it has the same size and type as image
	//		threshold1 – first threshold for the hysteresis procedure
	//		threshold2 – second threshold for the hysteresis procedure
	//		apertureSize – aperture size for the Sobel() operator
	//		L2gradient – flag indicating whether a more accurate L_2 norm = \sqrt{ (dI / dx) ^ 2 + (dI / dy) ^ 2 } should be used to calculate 
	//					the image gradient magnitude(L2gradient = true), or whether the default L_1 norm = | dI / dx | +| dI / dy | is enough(L2gradient = false).

	cv::Mat in;
	cv::Mat out;

	// use image 0 as input
	in = ImageToMat(0);

	cv::Canny(in, out, threshold1, threshold2, apertureSize, L2gradient);

	// place output in image 1
	MatToImage(out, 1);
}

void ImageProc::erodeCV(cv::Size ksize, cv::Point anchor, int iterations, int borderType, const cv::Scalar& borderValue)
{
	cv::Mat in;
	cv::Mat out;

	// use image 0 as input
	in = ImageToMat(0);

	// you can specify your own kernel, but i don't currently
	// have need for that
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, ksize);

	cv::erode(in, out, kernel, anchor, iterations, borderType, borderValue);

	// place output in image 1
	MatToImage(out, 1);
}

void ImageProc::dilateCV(cv::Size ksize, cv::Point anchor, int iterations, int borderType, const cv::Scalar& borderValue)
{
	cv::Mat in;
	cv::Mat out;

	// use image 0 as input
	in = ImageToMat(0);

	// you can specify your own kernel, but i don't currently
	// have need for that
	cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, ksize);

	cv::dilate(in, out, kernel, anchor, iterations, borderType, borderValue);

	// place output in image 1
	MatToImage(out, 1);
}

std::vector<std::vector<cv::Point>> ImageProc::findContoursCV(int mode, int method, cv::Point offset)
{
	// Find contours.
	//
	// Inputs:
	//		mode = RetrievalModes value
	//			RETR_EXTERNAL = 0	- retrieves only the extreme outer contours. It sets `hierarchy[i][2]=hierarchy[i][3]=-1`
	//								  for all the contours.
	//			RETR_LIST = 1		- retrieves all of the contours without establishing any hierarchical relationships.
	//			RETR_CCOMP = 2		- retrieves all of the contours and organizes them into a two-level hierarchy. At the top
	//								  level, there are external boundaries of the components. At the second level, there are
	//								  boundaries of the holes. If there is another contour inside a hole of a connected component, it
	//								  is still put at the top level.
	//			RETR_TREE = 3		- retrieves all of the contours and reconstructs a full hierarchy of nested contours.
	//			RETR_FLOODFILL = 4 
	//		method =  ContourApproximationModes
	//			CHAIN_APPROX_NONE = 1		- stores absolutely all the contour points. That is, any 2 subsequent points (x1,y1) and
	//										  (x2,y2) of the contour will be either horizontal, vertical or diagonal neighbors, that is,
	//										  max(abs(x1-x2),abs(y2-y1))==1.
	//			CHAIN_APPROX_SIMPLE = 2		- compresses horizontal, vertical, and diagonal segments and leaves only their end points.
	//										  For example, an up-right rectangular contour is encoded with 4 points. 
	//			CHAIN_APPROX_TC89_L1 = 3	- applies one of the flavors of the Teh-Chin chain approximation algorithm @cite TehChin89
	//			CHAIN_APPROX_TC89_KCOS = 4	- applies one of the flavors of the Teh-Chin chain approximation algorithm @cite TehChin89
	//		offset					- Optional offset by which every contour point is shifted. This is useful if the contours are 
	//								  extracted from the image ROI and then they should be analyzed in the whole image context. 
	// Outputs:
	//		return = vector of contours found
	//

	Q_UNUSED(mode);
	Q_UNUSED(method);
	Q_UNUSED(offset);

	std::vector<std::vector<cv::Point>> contours;		// return value
	std::vector<cv::Vec4i> hierarchy;

	// use image 0 as input
	cv::Mat in = ImageToMat(0);

	// input to findContours() must be single channel gray scale
	cv::cvtColor(in, in, cv::COLOR_BGR2GRAY);

	// make copy, findCountours() modifies image (may no longer be true)
	cv::Mat edgeCopy;
	in.copyTo(edgeCopy);
	findContours(edgeCopy, contours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);

	// Draw contours
	//cv::Mat drawing = cv::Mat::zeros(edgeCopy.size(), CV_8UC3);
	//for (int i = 0; i< contours.size(); ++i)
	//{
	//	cv::Scalar color = cv::Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
	//	drawContours(drawing, contours, i, color, 1, 8, hierarchy, 0, cv::Point());
	//}

	//// move image with contours drawn to output
	//MatToImage(drawing, 1);

	return contours;
}

cv::Mat ImageProc::ImageToMat(int imageIndex)
{
	// convert a native image to an OpenCV Mat
	//
	// inputs:
	//		imageIndex = target mp_Image[]
	// outputs:
	//		return = OpenCV Mat structure
	//

	assert(imageIndex >= 0 && imageIndex <= 2);

	int rowCount = GetHeight(imageIndex);
	int colCount = GetWidth(imageIndex);

	cv::Mat matRGBA(rowCount, colCount, CV_8UC4, mp_Image[imageIndex]->pData, cv::Mat::AUTO_STEP);
	cv::Mat matBGRA;

	//cv::cvtColor(matRGBA, matBGRA, cv::COLOR_RGBA2BGRA);

	//return matBGRA;
	return matRGBA;
}

void ImageProc::MatToImage(cv::Mat mat, int imageIndex)
{
	// convert OpenCV Mat to a native image
	//
	// inputs:
	//		mat = OpenCV Mat structure
	//		imageIndex = target mp_Image[]
	//

	assert(imageIndex >= 0 && imageIndex <= 2);
	assert(mat.rows > 0 && mat.cols > 0);

	RecreateImage(&mp_Image[imageIndex], mat.cols, mat.rows);

	switch (mat.channels()) {
	case 1:
	{
		UInt8* pData = mat.data;
		for (UInt16 y = 0; y < mat.rows; ++y)
		{
			for (UInt16 x = 0; x < mat.cols; ++x)
			{
				UInt8 b = *pData++;
				PixelType pix(b, b, b);
				SetPixel(imageIndex, x, y, pix);
			}
		}
		break;
	}
	case 3:
	{
		// RGB or BGR
		UInt8* pData = mat.data;
		for (UInt16 y = 0; y < mat.rows; ++y)
		{
			for (UInt16 x = 0; x < mat.cols; ++x)
			{
				UInt8 r = *pData++;
				UInt8 g = *pData++;
				UInt8 b = *pData++;
				PixelType pix(r, g, b, 255);
				SetPixel(imageIndex, x, y, pix);
			}
		}
		break;
	}
	case 4:
	{
		// if the input image has 4 8-bit channels, we may be 
		// able to just memcpy() it, but order still may not 
		// match, so for now, just assign pixels
		UInt8* pData = mat.data;
		for (UInt16 y = 0; y < mat.rows; ++y)
		{
			for (UInt16 x = 0; x < mat.cols; ++x)
			{
				UInt8 r = *pData++;
				UInt8 g = *pData++;
				UInt8 b = *pData++;
				UInt8 a = *pData++;
				PixelType pix(r, g, b, a);
				SetPixel(imageIndex, x, y, pix);
			}
		}
		break;
	}
	default:
		assert(false);
		break;
	}
}

bool ImageProc::CopyToClipboard(int imageIndex)
{
	// Copy image to clipboard
	//
	// Inputs:
	//		imageIndex = index of image to copy
	//
	// Outputs:
	//		return = true if data successfully copied
	//
//TODO:
//totally untested

	bool bReturn = false;

	QClipboard* pClipboard = QApplication::clipboard();
	if (pClipboard != nullptr)
	{
		ImageType* pImage = mp_Image[imageIndex];

		QImage image(pImage->Width, pImage->Height, QImage::Format_RGB32);
		image.fromData(pImage->pData, pImage->Width * pImage->Height);
			
		pClipboard->setImage(image);

		bReturn = true;
	}

	return bReturn;
}

bool ImageProc::PasteFromClipboard()
{
	// Replace the image with the contents of the clipboard.
	//
	// Return true if data successfully pasted
	//
//TODO:
//totally untested

	bool bReturn = false;

	const QClipboard* clipboard = QApplication::clipboard();
	const QMimeData* mimeData = clipboard->mimeData();

	if (mimeData->hasImage())
	{
		QImage* image = qvariant_cast<QImage*>(mimeData->imageData());
		if (image != nullptr)
		{
			Create(image->width(), image->height());
			for (int x = 0; x < image->width(); ++x)
			{
				for (int y = 0; y < image->height(); ++y)
				{
					PixelType pix;
					QRgb rgb = image->pixel(x, y);
					pix.SetRGBA((UInt8)qRed(rgb), (UInt8)qGreen(rgb), (UInt8)qBlue(rgb), 0xFF);
					SetPixel(0, x, y, pix);
				}
			}
			IdentityFilter();
		}
		bReturn = true;
	}

	return bReturn;
}
