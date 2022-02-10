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
using namespace android;
#define RAW_FILE "/storage/emulated/0/yuv_CABC.yuv"
#define MY_RAW_FILE "/data/dump/180_0_Layer_0_ybrsp_1920x1088_0.yuv"
#define RAW_YUV "/storage/emulated/0/12648_0_Layer_0_ybrsp_864x480_4216.yuv"
#define BACKLIGHT_SYSFS "/sys/class/backlight/sprd_backlight/brightness"
#define __CLASS__ "main"
bool isExit = false;

bool file_read(FILE *fp, ANativeWindow_Buffer &buffer) {
	char *source;
	size_t ret = -1;

	ssize_t bpr = buffer.stride * bytesPerPixel(buffer.format);
	switch (buffer.format) {
	case HAL_PIXEL_FORMAT_YCrCb_420_SP:
	case HAL_PIXEL_FORMAT_YCbCr_420_SP:
		bpr = buffer.stride * 1.5;
		break;
	case HAL_PIXEL_FORMAT_RGB_565:
		bpr = buffer.stride * 2;
		break;
	}
	size_t buf_len = bpr * buffer.height;
	ret = fread(buffer.bits, 1, buf_len, fp);
	char *buf = (char *) buffer.bits;

	return ret == buf_len;
}

class FrameData {
public:
	FrameData(FrameOutput *frm, FILE *f, DisplayEventReceiver *e) :
			frame(frm), fp(f), evt_receiver(e) {
	}
	FrameOutput *frame;
	FILE *fp;
	DisplayEventReceiver *evt_receiver;
};
static int write_int(const char* backlight_sysfs, int value) {
	int fd;
	fd = open(backlight_sysfs, O_RDWR);

	if (fd >= 0) {
		char buffer[20];
		int bytes = snprintf(buffer, sizeof(buffer), "%d\n", value);
		int amt = write(fd, buffer, bytes);

		close(fd);

		return amt == -1 ? -errno : 0;
	}

	return -errno;
}

int setBacklight(int value) {
	int fd;
//	fprintf(stderr, "set Backlight: %d\n", value);
	ALOGD("get brightness from YUV data: %d", value);
	fd = open(BACKLIGHT_SYSFS, O_RDWR);

	if (fd >= 0) {
		char buffer[20];
		int bytes = snprintf(buffer, sizeof(buffer), "%d\n", value);
		int amt = write(fd, buffer, bytes);

		close(fd);

		return amt == -1 ? -errno : 0;
	}

	return -errno;
}

int handler(int, int, void *data) {
	FrameData *framedata = (FrameData *) data;
	FrameOutput *frame = framedata->frame;
	DisplayEventReceiver* q = framedata->evt_receiver;

	ssize_t n;
	DisplayEventReceiver::Event buffer[1];
	static nsecs_t oldTimeStamp = 0;
	while ((n = q->getEvents(buffer, 1)) > 0) {
		for (int i = 0; i < n; i++) {
			if (buffer[i].header.type
					== DisplayEventReceiver::DISPLAY_EVENT_VSYNC) {
				frame->lock();
				ANativeWindow_Buffer& outBuffer = frame->outBuffer;
				if (!file_read(framedata->fp, outBuffer)) {
					isExit = true;
				}
				frame->unlockAndPost();
				char *buf = (char *) outBuffer.bits;
				setBacklight(buf[0]);
			}
			if (oldTimeStamp) {
				float t = float(buffer[i].header.timestamp - oldTimeStamp)
						/ s2ns(1);
				ALOGD(
						"frameNumber = %lu, %f ms (%f Hz)", frame->getSurface()->getNextFrameNumber(), t * 1000, 1.0 / t);
			}
			oldTimeStamp = buffer[i].header.timestamp;
		}
	}
	return 1;
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
	int width = 1280, height = 720;
	int outWidth = 0, outHeight = 0;
	char *path_name = NULL;

	while ((ch = getopt(argc, argv, "hf:i:o:")) != -1) {
		int value;
		switch (ch) {
		case 'f':
			ALOGD("input file name:%d %s\n", ch, optarg);
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
		case 'h':
			fprintf(stderr, "choose %c\n", ch);
			break;
		default:
			fprintf(stderr, "other option:%d %c\n", ch, ch);
			break;
		}
	}
	// sp<ProcessState> proc(ProcessState::self());
	// ProcessState::self()->startThreadPool();
	if (!path_name) {
		path_name = RAW_FILE;
	}
	FILE *fp = fopen(path_name, "rb");
	if (!fp) {
		fprintf(stderr, "read %s failed\n", path_name);
		return -1;
	}
	FrameOutput frame(true);

	if (outWidth == 0 || outHeight == 0) {
		outWidth = frame.getScreenWidth();
		outHeight = frame.getScreenHeight();
	}
	frame.setInputSurfaceInfo(width, height, HAL_PIXEL_FORMAT_YCbCr_420_SP);
	frame.setOutputSurfaceInfo(width, height, frame.getFormat());

	frame.initOutputFrame();
	frame.rotate(NATIVE_WINDOW_TRANSFORM_ROT_90).scalingToWindow();

	DisplayEventReceiver myDisplayEvent;
	FrameData data(&frame, fp, &myDisplayEvent);
	sp<Looper> loop = new Looper(false);
	loop->addFd(myDisplayEvent.getFd(), 0, ALOOPER_EVENT_INPUT, handler, &data);
	myDisplayEvent.setVsyncRate(3);

	do {
		int32_t ret = loop->pollOnce(-1);
		switch (ret) {
		case ALOOPER_POLL_TIMEOUT:
			printf("ALOOPER_POLL_TIMEOUT\n");
			break;
		case ALOOPER_POLL_ERROR:
			printf("ALOOPER_POLL_TIMEOUT\n");
			break;
		}
	} while (!isExit);
#if 0
	while (!isExit) {
		frame.lock();
		ANativeWindow_Buffer& outBuffer = frame.outBuffer;
		if (!file_read(fp, outBuffer)) {
			isExit = true;
		}
		frame.unlockAndPost();
		fprintf(stderr, "getNextFrameNumber %d\n",
			frame.getSurface()->getNextFrameNumber());
		usleep(41 * 1000);
	}
#endif
	fclose(fp);
	//	IPCThreadState::self()->joinThreadPool();
	return 0;
}
