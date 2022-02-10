#include "Feeder.h"
#include "PictureData.h"
#include "three_d_lut.h"

using namespace android;
#define LUT_TXT "/storage/emulated/0/3D_LUT_cal.txt"

#define __CLASS__ "Feeder"

ThreeDLut::ThreeDLut() {
	init_3d_lut(LUT_TXT);
}

ThreeDLut::~ThreeDLut() {
	LOGD("delete ThreeDLut");
	if (pt_lut)
		free(pt_lut);
}

int
ThreeDLut::init_3d_lut(const char *name) {
	FILE *fpt_lut = fopen(name, "rb");
	if (NULL == fpt_lut) {
		fprintf(stderr, "error open %s\n", name);
		return -ENOMEM;
	}
	pt_lut  = (unsigned int*)malloc(sizeof(unsigned int)*LUT_LVL*LUT_LVL*LUT_LVL);

	load_3d_lut(fpt_lut, pt_lut);

	fclose(fpt_lut);
	return 0;
}

int
ThreeDLut::fill_value(PictureData *pic, ANativeWindow_Buffer &buffer) {

	size_t Bpp = bytesPerPixel(buffer.format);
	ssize_t linelength =  buffer.stride * Bpp;
	uint8_t *base = (uint8_t *)buffer.bits;
	unsigned int R_i, G_i, B_i, R_o, G_o, B_o;
	unsigned int R_o_round, G_o_round, B_o_round;

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

			    R_i=base[(y * buffer.stride + x) * Bpp + 0 ];
			    G_i=base[(y * buffer.stride + x) * Bpp + 1 ];
			    B_i=base[(y * buffer.stride + x) * Bpp + 2 ];

			    tetrahedral_interpolation(R_i, G_i, B_i, &R_o, &G_o, &B_o, pt_lut);

			    //output RGB 8 bits
			    base[(y * buffer.stride + x) * Bpp + 0 ] = CLIP((ROUND_N(R_o, 2)), 255, 0);
			    base[(y * buffer.stride + x) * Bpp + 1 ] = CLIP((ROUND_N(G_o, 2)), 255, 0);
			    base[(y * buffer.stride + x) * Bpp + 2 ] = CLIP((ROUND_N(B_o, 2)), 255, 0);
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
		LOGE("Sorry, not support *%d* channels data", pic->mChannels);
		break;
	}

	return 0;
}