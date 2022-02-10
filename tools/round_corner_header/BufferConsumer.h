#ifndef  BUFFER_CONSUMER__H__
#define  BUFFER_CONSUMER__H__
#include <stdio.h>
#include <utils/String8.h>
#include <system/window.h>
#include <ui/PixelFormat.h>
#include <ui/Rect.h>

namespace android {

class BufferConsumer {
public:
	BufferConsumer();
	virtual ~BufferConsumer();
	virtual void onFrameAvaiable();
	inline bool checkDataValid() {
		return (mDataSize > 0) && (mData != nullptr);
	}
	void setDataBuffer(ANativeWindow_Buffer &outbuffer) {
		this->buffer = &outbuffer;
		size_t Bpp = bytesPerPixel(buffer->format);
		mDataSize = buffer->stride * Bpp * buffer->height;
		mData = (uint8_t *)buffer->bits;
	}

protected:
	uint8_t *mData;
	ssize_t mDataSize;
	ANativeWindow_Buffer *buffer;
};

class FileHeaderBuffer : public BufferConsumer {
public:
	FileHeaderBuffer();
	~FileHeaderBuffer();
	void onFrameAvaiable();
	void saveCropRawFile(const char *name, Rect &rect);
	void saveCropFile(const char *name, Rect &rect);

private:
	String8 result;
};

class BinaryBuffer : public BufferConsumer {
public:
	BinaryBuffer() {}
	~BinaryBuffer() {}
	void onFrameAvaiable() {};
};
}
#endif
