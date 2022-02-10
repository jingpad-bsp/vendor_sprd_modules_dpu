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
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/ISurfaceComposer.h>
#include <gui/DisplayEventReceiver.h>
#include "gralloc_public.h"
#include "FrameOutput.h"
#include "bmp.h"

using namespace android;
#define BMP_FILE "/storage/emulated/0/dota2_logo_32.bmp"
#define __CLASS__ "main"

bool file_read(FILE *fp, ANativeWindow_Buffer &buffer) {
	char *source;
	size_t ret = -1;

	uint8_t h = buffer.height;
	size_t Bpp = bytesPerPixel(buffer.format);
	void *base = buffer.bits;
	for (size_t y=0 ; y< buffer.height ; y++) {
		fread(base,  1, buffer.width * Bpp, fp);
		base = (void *)((char *)base + buffer.stride * Bpp);
	}

	return 0;
}

int get_bmp_param(FILE *fp, int *width, int *height, int *format) {
	BITMAPFILEHEADER fileHeader;
	BITMAPINFOHEADER infoHeader;

	fprintf(stderr, "Give me BMP 32bit picture\n");
	fread(&fileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
	fread(&infoHeader, sizeof(BITMAPINFOHEADER), 1, fp);
	fprintf(stderr, "width height (%d, %d)\n", infoHeader.biWidth, infoHeader.biHeight);
	fprintf(stderr, "width height (%d, %d)\n", infoHeader.biWidth, infoHeader.biHeight);
	*width = infoHeader.biWidth;
	*height = infoHeader.biHeight;
	if (infoHeader.biBitCount == 32) {
		*format = HAL_PIXEL_FORMAT_RGBA_8888;
	} else if (infoHeader.biBitCount == 24) {
		*format = HAL_PIXEL_FORMAT_RGB_888;
	}

	return 0;
}

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

int main(int argc, char** argv) {
	int ch;
	int width = 0, height = 0;
	int outWidth = 0, outHeight = 0;
	char *path_name = NULL;
	int in_format;
	int rotate_angle = 0;

	while ((ch = getopt(argc, argv, "hf:i:o:r:")) != -1) {
		int value;
		switch (ch) {
		case 'f':
			ALOGD("input picture file path:%d %s\n", ch, optarg);
			path_name = optarg;
			break;
		case 'i':
			fprintf(stderr, "input surface WIDTHxHEIGHT size: %s\n", optarg);

			check_opt2(optarg, &width, &height);
			break;
		case 'o':
			fprintf(stderr, "output surface WIDTHxHEIGHT size: %s\n", optarg);

			check_opt2(optarg, &outWidth, &outHeight);
			break;
		case 'r':
			fprintf(stderr, "rotate angle and mirror: %s\n", optarg);
			rotate_angle = atoi(optarg);
			break;
		case 'h':
			fprintf(stderr, "choose %c\n", ch);
			break;
		default:
			fprintf(stderr, "other option:%d %c\n", ch, ch);
			break;
		}
	}
	sp<ProcessState> proc(ProcessState::self());
	ProcessState::self()->startThreadPool();
	if (!path_name) {
		path_name = BMP_FILE;
	}
	FILE *fp = fopen(path_name, "rb");
	if (!fp) {
		fprintf(stderr, "read %s failed\n", path_name);
		return -1;
	}
	get_bmp_param(fp, &width, &height, &in_format);
	FrameOutput frame(true);

	if (outWidth == 0 || outHeight == 0) {
		outWidth = frame.getScreenWidth();
		outHeight = frame.getScreenHeight();
	}
	frame.setInputSurfaceInfo(width, height, in_format);
	frame.setOutputSurfaceInfo(width, height, in_format);

	frame.initOutputFrame();
	frame.rotate(rotate_angle);
	//frame.scalingToWindow();
	//frame.scalingTo(outWidth, outHeight);

	frame.lock();
	ANativeWindow_Buffer& outBuffer = frame.outBuffer;
	if (!file_read(fp, outBuffer)) {

	}

	frame.unlockAndPost();

	fclose(fp);
	fprintf(stderr, "\nPress Enter to close...\n");
	fgetc(stdin);

//	IPCThreadState::self()->joinThreadPool();
	return 0;
}
