#include "RawDataPicture.h"
#include <stdio.h>
#include <stdlib.h>
#include <utils/Log.h>
#include <cutils/memory.h>

#define __CLASS__ "RawDataPicture"


RawDataPicture::RawDataPicture() {}
RawDataPicture::~RawDataPicture() {}

int RawDataPicture::loadPicture(const char *path_name) {
	FILE *fp = fopen(path_name, "r");
	if (!fp) {
		LOGE("Error: load %s failed", path_name);
		return -1;
	}

	mData = (uint8_t *) malloc(mDataSize);
	if (!mData) {
		LOGE("malloc failed");
		isEmpty = false;
		return -2;
	}

	int bytesBylines = mChannels * mWidth;
#if 0
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

void RawDataPicture::setBmpParam(int width, int height, int channels) {

	LOGD("Give me Raw 32bit rawdata");

	mWidth = width;
	mHeight = height;
	mChannels = channels;


	mDataSize = ALIGN(mWidth, 16) * mChannels * mHeight;
	if (mDataSize) {
		isEmpty = true;
	}
	guessFormatFromChannels();
}
