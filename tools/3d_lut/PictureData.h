#ifndef PICTUREDATA_H_
#define PICTUREDATA_H_

#include <stdio.h>
#include "bmp.h"
#include "gralloc_public.h"
#include <system/graphics.h>
#include "debug.h"

namespace android {

class PictureData {
public:
	PictureData() {};
	virtual ~PictureData() {};

	virtual int loadPicture(const char *path_name);
	virtual bool isValid();
	inline int guessFormatFromChannels() {
			switch (mChannels){
			case 1:
				mFormat = 0;
				mFormat_string = "BMP 8 bit";
			case 3:
				mFormat = HAL_PIXEL_FORMAT_RGB_888;
				mFormat_string = "BMP R G B";
				break;
			case 4:
				mFormat = HAL_PIXEL_FORMAT_RGBA_8888;
				mFormat_string = "BMP R G B A";
				break;
			default:
				fprintf(stderr, "Error getBmpParam channels %d", mChannels);
				break;
			}
			return mFormat;
	}
	void dumpString();
	int inline getFormat() 	{ return mFormat; }
public:
	uint8_t *mData;
	ssize_t mDataSize;
	int mWidth;
	int mHeight;
	int mFormat;
	char *mFormat_string;
	int mChannels;
};

class BmpData :public PictureData {
public:
	BmpData();
	~BmpData();
	int loadPicture(const char *path_name);
	bool isValid() { return isEmpty; }
	void getBmpParam(FILE *fp);
private:
	bool isEmpty;
};


};
#endif // PICTUREDATA_H_
