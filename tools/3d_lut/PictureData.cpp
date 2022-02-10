#include "PictureData.h"
#include <stdio.h>
#include <stdlib.h>
#include <utils/Log.h>
#include <cutils/memory.h>

#define __CLASS__ "PictureData"
namespace android {

void PictureData::dumpString() {
		DDBG(mWidth);
		DDBG(mHeight);
		DDBG(mFormat);
		DDBG(mChannels);
		SDBG(mFormat_string);
	}

BmpData::BmpData(): isEmpty(true) {
	mData = nullptr;
	LOGD("init a Bmpdata");
	}
BmpData::~BmpData() {
	LOGD("destruct Bmpdata");
	if (mData) {
		free(mData);
	}
	isEmpty = true;
}

void BmpData::getBmpParam(FILE *fp) {
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;

	LOGD("Give me BMP 32bit picture");
	fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
	fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, fp);
	LOGD("width height (%d, %d)", infoHeader.biWidth, infoHeader.biHeight);

	mWidth = infoHeader.biWidth;
	mHeight = infoHeader.biHeight;
	mChannels = infoHeader.biBitCount / 8;


	mDataSize = infoHeader.biSizeImage;
	if (mDataSize) {
		isEmpty = true;
	}
	guessFormatFromChannels();

	LOGD("seek to data offset: fileHeader.bfOffBits %d", fileHeader.bfOffBits);
	fseek(fp, fileHeader.bfOffBits, SEEK_SET);
}

int BmpData::loadPicture(const char *path_name) {
	SDBG(path_name);
	FILE *fp = fopen(path_name, "r");
	if (!fp) {
		LOGE("Error: load %s failed", path_name);
		return -1;
	}

	getBmpParam(fp);
	mData = (uint8_t *) malloc(mDataSize);
	if (!mData) {
		LOGE("malloc failed");
		isEmpty = false;
		return -2;
	}

	int bytesBylines = mChannels * mWidth;
#if 1
	for (int i = mHeight - 1; i >= 0; i--) {
		uint8_t *base = mData + bytesBylines * (mHeight - 1 - i);
		fread(mData + bytesBylines * i, 1, bytesBylines, fp);
		//android_memset32((uint32_t*)base, 0xFFFF0000, mWidth * mChannels);
	}
#else
	for (int i = 0; i < mHeight; i++) {
		uint8_t *base = mData + bytesBylines * i;
		fread(mData + bytesBylines * i, 1, bytesBylines, fp);
		//android_memset32((uint32_t*)base, 0xFFFF0000, mWidth * mChannels);
	}
#endif
	fclose(fp);
	return 0;
}
};
