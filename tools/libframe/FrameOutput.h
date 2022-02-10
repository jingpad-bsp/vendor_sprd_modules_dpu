/*
 * FrameOutput.h
 *
 *  Created on: May 30, 2018
 *      Author: infichen
 */

#ifndef FRAMEOUTPUT_H_
#define FRAMEOUTPUT_H_
#include <cutils/memory.h>
#include <stdio.h>
#include <utils/Log.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
#include <system/window.h>
#include <ui/DisplayInfo.h>
#include <gui/Surface.h>
#include <gui/SurfaceComposerClient.h>
#include <gui/ISurfaceComposer.h>
#include "gralloc_public.h"
#include "debug.h"
namespace android {

class FrameOutput {
public:
	FrameOutput();
	FrameOutput(bool displayOnly);
	virtual ~FrameOutput();

	FrameOutput& rotate(int transform);
	FrameOutput& scalingToWindow();
	FrameOutput& scalingTo(int toWidth, int toHeight);

	void setInputSurfaceInfo(int width, int height, PixelFormat format);
	void setOutputSurfaceInfo(int width, int height, PixelFormat format);
	void initOutputFrame(int32_t flags = 0);
	ssize_t bytesPerRow();


	PixelFormat getFormat() const {
		return format;
	}
	inline int getScreenWidth() const {
		return mDisplayInfo.w;
	}
	inline int getScreenHeight() const {
		return mDisplayInfo.h;
	}
	ANativeWindow_Buffer& getWindowBuffer() {
		return outBuffer;
	}
	DisplayInfo& getDisplayInfo() {
		return mDisplayInfo;
	}
	sp<Surface>& getSurface() {
		return mSurface;
	}
	void lock();
	void unlockAndPost();
	inline void setPosition(float x, float y) {
		dest_x = x;
		dest_y = y;
	}
	inline void setCrop(float x, float y) {
		source_x = x;
		source_y = y;
	}
private:
	bool mDisplayOnly;
	sp<SurfaceComposerClient> mClient;
	PixelFormat format;
	sp<SurfaceControl> mSurfaceControl;
	sp<Surface> mSurface;
	int width;
	int height;
	int stride;

	int outWidth;
	int outHeight;
	int outFormat;

	float source_x, source_y;
	float dest_x, dest_y;
	DisplayInfo mDisplayInfo;
	SurfaceComposerClient::Transaction mTrans;

public:
	ANativeWindow_Buffer outBuffer;
};

} /* namespace android */
#endif /* FRAMEOUTPUT_H_ */
