/*
 * FrameOutput.cpp
 *
 *  Created on: May 30, 2018
 *      Author: infichen
 */

#include "FrameOutput.h"

#define __CLASS__ "FrameOutput"


namespace android {

FrameOutput::FrameOutput() {

}
FrameOutput::FrameOutput(bool displayOnly) :
		mClient(nullptr), mDisplayOnly(displayOnly),
		source_x(0.0f), source_y(0.0f), dest_x(0.0f), dest_y(0.0f) {
	mClient = new SurfaceComposerClient();
	sp<IBinder> mainDpy = SurfaceComposerClient::getInternalDisplayToken();
	SurfaceComposerClient::getDisplayInfo(mainDpy, &mDisplayInfo);

	setOutputSurfaceInfo(mDisplayInfo.w, mDisplayInfo.h, HAL_PIXEL_FORMAT_RGBA_8888);
}

FrameOutput::~FrameOutput() {
	fprintf(stderr, "release Frameout Surface\n");
	if (mDisplayOnly) {
		sp<IBinder> mainDpy = SurfaceComposerClient::getInternalDisplayToken();
		SurfaceComposerClient::Transaction t;
		t.setDisplayLayerStack(mainDpy, 0); // default stack
		t.apply(true);
	}
}

FrameOutput& FrameOutput::rotate(int transform) {
	auto anw = static_cast<ANativeWindow*>(mSurface.get());
	switch (transform) {
	case NATIVE_WINDOW_TRANSFORM_ROT_90:
	case NATIVE_WINDOW_TRANSFORM_ROT_270:
	{
		//native_window_set_buffers_transform(anw, transform);
		int t = outWidth;
		outWidth = outHeight;
		outHeight = t;
		DDBG(outWidth);
		DDBG(outHeight);
		mTrans.setSize(mSurfaceControl, outWidth, outHeight);
		mTrans.setGeometryAppliesWithResize(mSurfaceControl);
		break;
	}
	case 0:
	case NATIVE_WINDOW_TRANSFORM_ROT_180:
	case NATIVE_WINDOW_TRANSFORM_FLIP_H:
	case NATIVE_WINDOW_TRANSFORM_FLIP_V:
		break;
	default:
		LOGE("Error rotate angle: %d\n", transform);
		return *this;
	}
	native_window_set_buffers_transform(anw, transform);

	return *this;
}

FrameOutput& FrameOutput::scalingToWindow() {

	native_window_set_scaling_mode(mSurface.get(), NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
	mTrans.setSize(mSurfaceControl, mDisplayInfo.w, mDisplayInfo.h);
	return *this;
}

FrameOutput& FrameOutput::scalingTo(int toWidth, int toHeight) {
	if (toWidth < 1 || toHeight < 1) {
		LOGE("Error scaingTo size (%dx%d)\n", toWidth, toHeight);
		return *this;
	}
	outWidth = toWidth;
	outHeight = toHeight;
	native_window_set_scaling_mode(mSurface.get(), NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);
	mTrans.setSize(mSurfaceControl, outWidth, outHeight);
	return *this;
}


void FrameOutput::setInputSurfaceInfo(int width, int height,
		PixelFormat format) {
	this->width = width;
	this->height = height;
	this->format = format;
}

void FrameOutput::setOutputSurfaceInfo(int width, int height,
		PixelFormat format) {
	this->outWidth = width;
	this->outHeight = height;
	this->outFormat = format;
}

void FrameOutput::initOutputFrame(int32_t flags) {
	DDBG(width);
	DDBG(height);
	XDBG(format);

     //flags: ISurfaceComposerClient::eNonPremultiplied;

	mSurfaceControl = mClient->createSurface(String8("show_picture"), width, height,
			outFormat, flags);

	mSurface = mSurfaceControl->getSurface();

    native_window_set_buffers_geometry(mSurface.get(),
            width,
            height,
            format);

	// Get main display parameters.
	sp<IBinder> mainDpy = SurfaceComposerClient::getInternalDisplayToken();
	SurfaceComposerClient::getDisplayInfo(mainDpy, &mDisplayInfo);

	int layer_stack = mDisplayOnly? 1: 0;
	SurfaceComposerClient::Transaction &t = mTrans;
	t.setDisplayLayerStack(mainDpy, layer_stack); // default stack
	t.setLayer(mSurfaceControl, 200000);
	t.setAlpha(mSurfaceControl, 1.0f);
	if (dest_y || dest_x) {
		LOGD("set position (%0.2f, %0.2f)", dest_x, dest_y);
		t.setPosition(mSurfaceControl,dest_x, dest_y);
	}
	t.setLayerStack(mSurfaceControl, layer_stack);
}

ssize_t FrameOutput::bytesPerRow() {
	ssize_t bpr = outBuffer.stride * bytesPerPixel(outBuffer.format);
	switch (outBuffer.format) {
	case HAL_PIXEL_FORMAT_YCrCb_420_SP:
	case HAL_PIXEL_FORMAT_YCbCr_420_SP:
	case HAL_PIXEL_FORMAT_RGB_565:
		bpr = outBuffer.stride * 2;
		break;
	}
	return bpr;
}
void FrameOutput::lock() {
	DDBG(outWidth);
	DDBG(outHeight);
	XDBG(outFormat);
	mTrans.apply(true);
	int ret = mSurface->lock(&outBuffer, NULL);
	LOGE("lock result: ret = %d (%s)", ret, strerror(-ret));
}
void FrameOutput::unlockAndPost() {
	int ret = mSurface->unlockAndPost();
	LOGE("unlockAndPost result: ret = %d, (%s)", ret, strerror(-ret));
}

} /* namespace android */
