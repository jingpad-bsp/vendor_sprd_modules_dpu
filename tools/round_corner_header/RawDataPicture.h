#ifndef RAW_DATA_PICTURE__H__
#define RAW_DATA_PICTURE__H__

#include "PictureData.h"
using namespace android;

class RawDataPicture :public PictureData {
public:
	RawDataPicture();
	~RawDataPicture();
	int loadPicture(const char *path_name);
	bool isValid() { return isEmpty; }
	void setBmpParam(int width, int height, int channels);
private:
	bool isEmpty;
};


#endif
