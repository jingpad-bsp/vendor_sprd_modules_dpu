#include <stdio.h>
#include <stdlib.h>
#include "BufferConsumer.h"
#include "PictureData.h"

#define __CLASS__ "FileHeaderBuffer"
namespace android {
BufferConsumer::BufferConsumer() {}
BufferConsumer::~BufferConsumer() {}

FileHeaderBuffer::FileHeaderBuffer() {
	result.clear();
}

FileHeaderBuffer::~FileHeaderBuffer() {

}

void FileHeaderBuffer::onFrameAvaiable() {
	size_t Bpp = bytesPerPixel(buffer->format);

	if (!checkDataValid()) {
		LOGE("Error: data or size invalid");
		return;
	}

}

void FileHeaderBuffer::saveCropRawFile(const char *name, Rect &rect) {
	int Bpp = bytesPerPixel(buffer->format);
	uint8_t *base = (uint8_t *)buffer->bits;


	FILE *fp =  fopen(name, "wb");
	if (!fp) {
		LOGD("create %s failed", name);
		return;
	}

	ssize_t cnt = 0;
	for (int i = 0; i < buffer->height; i++)
		for (int j = 0;j < buffer->width; j++) {
			if ( j >= rect.left && j <= rect.right && i >= rect.top && i <= rect.bottom) {
				uint8_t *value = base + (i * buffer->stride + j) * Bpp;
				cnt += fwrite(value, 1, (int)Bpp, fp);
			}
		}

	LOGD("Save Crop File %s, %ld", name, cnt);
	fclose(fp);
}

void FileHeaderBuffer::saveCropFile(const char *name, Rect &rect) {

	int Bpp = bytesPerPixel(buffer->format);
	uint8_t *base = (uint8_t *)buffer->bits;

	result.clear();
	FILE *fp = fopen(name, "w");
	if (!fp) {
		LOGD("create %s failed", name);
		return;
	}

	for (int i = 0; i < buffer->height; i++)
		for (int j = 0;j < buffer->width; j++) {
			if ( j >= rect.left && j <= rect.right && i >= rect.top && i <= rect.bottom) {
				uint8_t *value = base + (i * buffer->stride + j) * Bpp;
				if (j % 2 == 0)
					result.append("\n");
				else
					result.append(" ");
				result.appendFormat("0x%02X, 0x%02X, 0x%02X, 0x%02X,", value[0], value[1], value[2], value[3]);
			}
		}

	fwrite(result.c_str(), result.size(), 1, fp);
	LOGD("Save Crop File %s, %ld", name, result.size());
	fclose(fp);
}

};
