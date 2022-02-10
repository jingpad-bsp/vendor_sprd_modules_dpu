#ifndef _FEEDER_H_
#define _FEEDER_H_

#include "PictureData.h"
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/ISurfaceComposer.h>

using namespace android;

class Feeder {
public:
	Feeder() {}
	virtual ~Feeder() {}
	virtual	int fill_value(PictureData *pic, ANativeWindow_Buffer &buffer);
};

class ThreeDLut: Feeder {
public:
	ThreeDLut();
	~ThreeDLut();
	int fill_value(PictureData *pic, ANativeWindow_Buffer &buffer);
	int init_3d_lut(const char *name);
private:
	unsigned int *pt_lut;
};

class BmpFeeder: Feeder {
public:
	BmpFeeder() {};
	~BmpFeeder() {};
	int fill_value(PictureData *pic, ANativeWindow_Buffer &buffer) {
		size_t Bpp = bytesPerPixel(buffer.format);
		ssize_t linelength =  buffer.stride * Bpp;
		uint8_t *base = (uint8_t *)buffer.bits;

		switch (pic->mChannels) {
		case 3:
			for (int y = 0; y < buffer.height; y++) {
				for (int x = 0; x < buffer.width; x++) {
					//R
					base[(y * buffer.stride + x) * Bpp + 0 ] = pic->mData[(y * pic->mWidth + x) * pic->mChannels + 2];
					//G
					base[(y * buffer.stride + x) * Bpp + 1 ] = pic->mData[(y * pic->mWidth + x) * pic->mChannels + 1];
					//B
					base[(y * buffer.stride + x) * Bpp + 2 ] = pic->mData[(y * pic->mWidth + x) * pic->mChannels + 0];
					//A
					base[(y * buffer.stride + x) * Bpp + 3 ] = 0xff;
				}
			}
			break;
		case 4:
		// 4 channels for debug, bmp 32bit is not support transfrom to c header file.
			for (int y = 0; y < buffer.height; y++) {
				for (int x = 0; x < buffer.width; x++) {
					base[(y * buffer.stride + x) * Bpp + 0 ] = pic->mData[(y * pic->mWidth + x) * pic->mChannels + 0];
					base[(y * buffer.stride + x) * Bpp + 1 ] = pic->mData[(y * pic->mWidth + x) * pic->mChannels + 1];
					base[(y * buffer.stride + x) * Bpp + 2 ] = pic->mData[(y * pic->mWidth + x) * pic->mChannels + 2];

					base[(y * buffer.stride + x) * Bpp + 3 ] = pic->mData[(y * pic->mWidth + x) * pic->mChannels + 3];
					//base[(y * buffer.stride + x) * Bpp + 3 ] = 0xff;
				}
			}
			break;
		default:
			ALOGE("Sorry, not support *%d* channels data", pic->mChannels);
			break;
		}
		return 0;
	}
};

#endif