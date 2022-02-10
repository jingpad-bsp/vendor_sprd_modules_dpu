/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cutils/memory.h>
#include <stdio.h>
#include <utils/Log.h>
#include <unistd.h>
#include <android/looper.h>

#include <utils/Looper.h>
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <system/window.h>
#include "gralloc_public.h"
#include "PictureData.h"
#include "RawDataPicture.h"
#include "FrameOutput.h"
#include "BufferConsumer.h"

#define __CLASS__ "main"
using namespace android;
#define BMP_FILE "/storage/emulated/0/dota2_logo_32.bmp"

bool isExit = false;
uint32_t g_mem_data = 0xFF0000FF;

int fill_value_round_corner(PictureData *pic, ANativeWindow_Buffer &buffer) {

	size_t Bpp = bytesPerPixel(buffer.format);
	ssize_t linelength =  buffer.stride * Bpp;
	uint8_t *base = (uint8_t *)buffer.bits;

	switch (pic->mChannels) {
	case 3:
		for (int y = 0; y < buffer.height; y++) {
			for (int x = 0; x < buffer.width; x++) {
				base[(y * buffer.stride + x) * Bpp + 0 ] = 0;
				base[(y * buffer.stride + x) * Bpp + 1 ] = 0;
				base[(y * buffer.stride + x) * Bpp + 2 ] = 0;

				base[(y * buffer.stride + x) * Bpp + 3 ] = pic->mData[(y * pic->mWidth + x) * pic->mChannels + 0];
			}
		}
		break;
	case 4:
	// 4 channels for debug, bmp 32bit is not support transfrom to c header file.
		for (int y = 0; y < buffer.height; y++) {
			for (int x = 0; x < buffer.width; x++) {
				base[(y * buffer.stride + x) * Bpp + 0 ] = 0;
				base[(y * buffer.stride + x) * Bpp + 1 ] = 0;
				base[(y * buffer.stride + x) * Bpp + 2 ] = 0;

				base[(y * buffer.stride + x) * Bpp + 3 ] = pic->mData[(y * pic->mWidth + x) * pic->mChannels + 0];
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

enum {
	FILE_TYPE_BMP,
	FILE_TYPE_RAW,
};

int check_opt2(char *optarg, int *val0, int *val1)
{
	char *token;
	char delim[] = "xX*";
	int i;

	for (token = strsep(&optarg, delim), i = 0; token != NULL; token =
			strsep(&optarg, delim), i++) {
		int value = atoi(token);

		if (value < 0) {
			LOGE("Failed:  check -i option param, token %s\n", token);
			return -1;
		}
		if (i == 0)
			*val0 = value;
		else
			*val1 = value;
	}

	return 0;
}

void usage() {
	const char *usage_string="Usage: round_corner <option> <...>\n\n\
		Option:\n\
		-f  Bitmap picture, the pattern picture\n\
		\n\
		-b  raw data of the bitmap or the screen, format is ARGB8888\n\
			width is not aligned to 16.	\n\
		\n\
		-i  input source crop, means where position data get from \n\
			 -i  1x1 means get data from point (1, 1) \n\
		\n\
		-o  output dest frame position, means display position on the screen \n\
		\n\
		-r  set raw data WIDTHxHEIGHT \n\n\
		-R  set round corner radius, such as top/bottom header height, default value is 72. \n\n\
		-h  Help string.";
	fprintf(stderr, "%s\n", usage_string);
}

int main(int argc, char** argv) {
	int ch;
	int in_x = 0, in_y = 0;
	int out_x = 0, out_y = 0;
	int raw_width = 540,raw_height = 1132;
	int radius = 72;
	char *path_name = NULL;
	int file_type = 0;

	while ((ch = getopt(argc, argv, "hpf:i:o:b:d:r:R:")) != -1) {
		int value;
		switch (ch) {
		case 'f':
			LOGD("input picture file path:%d %s\n", ch, optarg);
			path_name = optarg;
			file_type = FILE_TYPE_BMP;
			break;
		case 'i':
			LOGD("input surface position: %s\n", optarg);
			check_opt2(optarg, &in_x, &in_y);
			break;
		case 'o':
			LOGD("output surface pos: %s\n", optarg);
			check_opt2(optarg, &out_x, &out_y);
			break;
		case 'b':
			path_name = optarg;
			file_type = FILE_TYPE_RAW;
			break;
		case 'r':
			LOGD("raw data resolution: %s\n", optarg);
			check_opt2(optarg, &raw_width, &raw_height);
			break;
		case 'R':
			LOGD("radius set to %s", optarg);
			radius = atoi(optarg);
			break;
		case 'h':
			usage();
			return 0;
		case 'p':
			break;

		case 'd':
			sscanf(optarg, "0x%x", &g_mem_data);
		default:
			LOGD("other option:%d %c\n", ch, ch);
			return 0;
		}
	}
	ALOGD("\n\n ---------------- start --------------------");
	sp<ProcessState> proc(ProcessState::self());
	ProcessState::self()->startThreadPool();

	PictureData *pic = nullptr;
	BmpData bmpdata;
	RawDataPicture rawdata;

	switch (file_type) {
	case FILE_TYPE_BMP:
		pic = &bmpdata;
		break;
	case FILE_TYPE_RAW:
		pic = &rawdata;
		rawdata.setBmpParam(raw_width, raw_height, 4);
		break;
	default:
		LOGE("Error file type %d", file_type);
		pic = nullptr;
		return -5;
	}
	DDBG(file_type);
	if (!path_name)
		path_name = BMP_FILE;

	pic->loadPicture(path_name);
	pic->dumpString();
	if (!pic->isValid()) {
		LOGE("Error load picture");
		return 0;
	}
	FrameOutput  frame(false);
	int format = HAL_PIXEL_FORMAT_RGBA_8888;
	if (pic->mChannels == 4) {
		format = HAL_PIXEL_FORMAT_RGBA_8888;
	}

	// round_corner need ARGB format, so we need dequeue a ARGB surface.
	frame.setInputSurfaceInfo(pic->mWidth, pic->mHeight, format);
	// user default screen resolution.
	frame.setOutputSurfaceInfo(pic->mWidth, pic->mHeight, HAL_PIXEL_FORMAT_RGBA_8888);
	frame.setPosition(out_x, out_y);
	frame.initOutputFrame();
	//frame.rotate(NATIVE_WINDOW_TRANSFORM_ROT_90).scalingToWindow();
	//frame.scalingToWindow();

	frame.lock();
	ANativeWindow_Buffer outBuffer = frame.getWindowBuffer();
	uint8_t h = outBuffer.height;

	size_t Bpp = bytesPerPixel(outBuffer.format);
	void *base = outBuffer.bits;
	DDBG(outBuffer.format);
	DDBG(outBuffer.height);
	DDBG(Bpp);

	fill_value_round_corner(pic, outBuffer);

	frame.unlockAndPost();
	if (file_type != FILE_TYPE_RAW) {
		BufferConsumer *bc;
		FileHeaderBuffer fileHeader;
		bc = &fileHeader;
		bc->setDataBuffer(outBuffer);
		bc->onFrameAvaiable();
		//Rect  full_rect(0, 0, outBuffer.stride, outBuffer.height);

		Rect  top_rect(0, 0, raw_width -1  , radius -1);
		fileHeader.saveCropFile("/sdcard/lcd_top_corner.h", top_rect);
		Rect bottom_rect(0, raw_height -1 - (radius - 1), raw_width - 1, raw_height - 1);
		fileHeader.saveCropFile("/sdcard/lcd_bottom_corner.h", bottom_rect);

		fileHeader.saveCropRawFile("/sdcard/raw.bin", top_rect);

	}
	LOGD("Done!");
	IPCThreadState::self()->joinThreadPool();

	ALOGD("---------------- end --------------------\n\n");
	return 0;
}
