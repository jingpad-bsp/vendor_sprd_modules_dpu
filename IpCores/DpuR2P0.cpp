#include "IpCores.h"


// @Override
void DpuR2P0::cleanAccelerator(SprdHWLayer **list, int count)
{
	if (list == NULL) {
		ALOGE("DpuR2P0 cleanAccelerator list is NULL\n");
		return ;
	}

	for (int i = 0; i < count; i++) {
		if (list[i] == NULL) {
			ALOGE("DpuR2P0 cleanAccelerator list[%d] is NULL\n",i);
			return ;
		}
		list[i]->setLayerAccelerator(ACCELERATOR_NON);
	}

	ALOGI_IF(mDebugFlag, "dpur2p0 cleanAccelerator: layer(%d)", count);
}

// @Override
bool DpuR2P0::checkAFBCBlockSize(SprdHWLayer *l)
{
	if (l == NULL)
		return false;

	const native_handle_t *pNativeHandle = l->getBufferHandle();

	if (pNativeHandle == NULL)
		return false;

	ALOGI_IF(mDebugFlag, "dpur2p0 checkAFBCBlockSize: compressed(%d),"
		"format(%d), pitch(%d), vstride(%d)",
		ADP_COMPRESSED(pNativeHandle), ADP_FORMAT(pNativeHandle),
		ADP_STRIDE(pNativeHandle), ADP_VSTRIDE(pNativeHandle));

	if (ADP_COMPRESSED(pNativeHandle)) {
		switch(ADP_FORMAT(pNativeHandle)) {
			/* RGB888 afbc align 16x16 */
			case HAL_PIXEL_FORMAT_RGBA_8888:
			case HAL_PIXEL_FORMAT_RGBX_8888:
			case HAL_PIXEL_FORMAT_BGRA_8888:
				if (((ADP_STRIDE(pNativeHandle) % 8) == 0) &&
					((ADP_VSTRIDE(pNativeHandle) % 8) == 0))
					return true;
				break;
			/* RGB565 afbc align 16x8 */
			case HAL_PIXEL_FORMAT_RGB_565:
				if (((ADP_STRIDE(pNativeHandle) % 16) == 0) &&
					((ADP_VSTRIDE(pNativeHandle) % 8) == 0))
					return true;
				break;
			default:
				break;
		}

		ALOGD("dgw Dpu checked invalid handle size under AFBC: pitch: %d, vstride:%d",
			ADP_STRIDE(pNativeHandle), ADP_VSTRIDE(pNativeHandle));
		return false;
	}

	return true;
}

// @Override
bool DpuR2P0::checkBlendMode(SprdHWLayer *l)
{
	if (l == NULL)
		return false;

	ALOGI_IF(mDebugFlag, "dpur2p0 checkBlendMode: BlendMode(0x%x), PlaneAlpha(0x%x)",
		l->getBlendMode(), l->getPlaneAlpha());

	/* When the blend mode is HWC_BLENDING_PREMULT,
		the layer alpha must be 0xff. */
	if (l->getBlendMode() == HWC_BLENDING_PREMULT) {
		if (l->getPlaneAlpha() != 0xFF) {
			return false;
		}
	}

	return true;
}


// @Override
bool DpuR2P0::checkTransform(SprdHWLayer *l)
{
	if (l == NULL)
		return false;

	const native_handle_t *pNativeHandle = l->getBufferHandle();

	ALOGI_IF(mDebugFlag, "dpur2p0 checkTransform: Transform(%d), SRCRect(%d,%d)",
		l->getTransform(), l->getSprdSRCRect()->x, l->getSprdSRCRect()->y);

	if (l->getTransform() != 0) {
		/* DPU just support ARGB8888,XFBC-ARGB8888,YUV420 rotation. */
		switch(ADP_FORMAT(pNativeHandle)) {
			case HAL_PIXEL_FORMAT_RGBA_8888:
			case HAL_PIXEL_FORMAT_RGBX_8888:
			case HAL_PIXEL_FORMAT_BGRA_8888:
				break;
			case HAL_PIXEL_FORMAT_YCbCr_420_SP:
			case HAL_PIXEL_FORMAT_YCrCb_420_SP:
			case HAL_PIXEL_FORMAT_YV12:
			case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
				if (ADP_COMPRESSED(pNativeHandle)) {
					return false;
				}
				break;
			default:
				return false;
		}

		/* The crop starting coordinates of the input rotation
			must be (0,0). */
		if ((l->getSprdSRCRect()->x != 0) ||
			(l->getSprdSRCRect()->y != 0)) {
			return false;
		}
	}

	return true;
}

// @Override
bool DpuR2P0::checkLayerFormat(SprdHWLayer *l)
{
	if (l == NULL)
		return false;

	const native_handle_t *pNativeHandle = l->getBufferHandle();

	if (pNativeHandle == NULL)
		return false;

	ALOGI_IF(mDebugFlag, "dpur2p0 checkLayerFormat: Format(%d)",
		ADP_FORMAT(pNativeHandle));

	switch (ADP_FORMAT(pNativeHandle)) {
		case HAL_PIXEL_FORMAT_RGBA_8888:
		case HAL_PIXEL_FORMAT_RGBX_8888:
		case HAL_PIXEL_FORMAT_BGRA_8888:
		case HAL_PIXEL_FORMAT_RGB_565:
		case HAL_PIXEL_FORMAT_YCbCr_420_SP:
		case HAL_PIXEL_FORMAT_YCrCb_420_SP:
		case HAL_PIXEL_FORMAT_YV12:
		case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
			return true;
			break;
		default:
			break;
	}

	return false;
}

// @Override
bool DpuR2P0::checkScaleSize(SprdHWLayer *l)
{
	if (l == NULL)
		return false;

	uint32_t tform = l->getTransform();

	ALOGI_IF(mDebugFlag, "dpur2p0 checkScaleSize: Transform(%d), SRCRect(%d,%d), FBRect(%d,%d)",
		tform, l->getSprdSRCRect()->w, l->getSprdSRCRect()->h,
		l->getSprdFBRect()->w, l->getSprdFBRect()->h);

	if (tform & HAL_TRANSFORM_ROT_90) {
		if ((l->getSprdSRCRect()->w != l->getSprdFBRect()->h) ||
			(l->getSprdSRCRect()->h != l->getSprdFBRect()->w)) {
			return false;
		}
	}
	else if ((l->getSprdSRCRect()->w != l->getSprdFBRect()->w) ||
		(l->getSprdSRCRect()->h != l->getSprdFBRect()->h)) {
		return false;
	}

	return true;
}

#define LAYER_NUM 8
// @Override
bool DpuR2P0::isMaxOverlayerOutOfLimit(SprdHWLayer **list, int count)
{
	struct sprdRect layer, layers[LAYER_NUM];
	int maxX, maxY, minX, minY;
	int maxs[LAYER_NUM];
	int i, j;
	const int limitNum = 4;
	SprdHWLayer *l;

	/* when corner is set :
	 * 	count + 2 > LAYER_NUM, dpu cannot support
	 * 	count < limitNum, no need to calculate
	 * when corner is not set:
	 * 	cout > LAYER_NUM, dpu cannot support
	 * 	count <= limitNum, no need to calculate
	 */
	if (mCornerSize) {
		if (count > (LAYER_NUM - 2))
			return false;
		if (count < limitNum)
			return true;
	} else {
		if (count > LAYER_NUM)
			return false;
		if (count < (limitNum + 1))
			return true;
	}

	for (i = 0; i < count; i++) {
		l = list[i];

		layers[i].x = l->getSprdFBRect()->x;
		layers[i].y = l->getSprdFBRect()->y;
		layers[i].w = l->getSprdFBRect()->w;
		layers[i].h = l->getSprdFBRect()->h;
	}

	if (mCornerSize) {
		int height, width, radius;

		height = mDispHeight;
		width = mDispWidth;
		radius = mCornerSize;

		layers[count].x = 0;
		layers[count].y = 0;
		layers[count].w = width;
		layers[count].h = radius;
		count++;

		layers[count].x = 0;
		layers[count].y = height - radius;
		layers[count].w = width;
		layers[count].h = radius;
		count++;
	}

	for (i = 0; i < count; i++) {
		layer.x = layers[i].x;
		layer.y = layers[i].y;
		layer.w = layers[i].w;
		layer.h = layers[i].h;
		maxs[i] = 1;

		for (j = 0; j < count; j++) {
			if (layer.x + layer.w > layers[j].x &&
				layers[j].x + layers[j].w > layer.x &&
				layer.y + layer.h > layers[j].y &&
				layers[j].y + layers[j].h > layer.y &&
				i != j) {
				maxX = std::max(layers[i].x, layers[j].x);
				maxY = std::max(layers[i].y, layers[j].y);
				minX = std::min(layers[i].x + layers[i].w,
					layers[j].x + layers[j].w);
				minY = std::min(layers[i].y + layers[i].h,
					layers[j].y + layers[j].h);

				layer.x = maxX;
				layer.y = maxY;
				layer.w = minX - maxX;
				layer.h = minY - maxY;

				maxs[i]++;
			}
		}
	}

	for (i = 1; i < count; i++) {
		if (maxs[i] > 4)
			return false;
	}

	return true;
}

// @Override
int DpuR2P0::prepare(SprdHWLayer **list, int count, bool *support, int dpu_limit)
{
	HWC_IGNORE(support);
	SprdHWLayer *l;

	if (count > dpu_limit) {
		ALOGW("Warning: Dispc cannot support %d layer blending(%d)", count, *support);
		return -1;
	}

	ALOGI_IF(mDebugFlag, "dpur2p0 prepare: count(%d)", count);

	if (!isMaxOverlayerOutOfLimit(list, count))
		return -1;

	for (int i = 0; i < count; i++) {
		l = list[i];
		if (l == NULL)
			return -1;

		/* Check blend mode */
		if (!checkBlendMode(l)) {
			cleanAccelerator(list, count);
			break;
		}

		/* check scaling */
		if (!checkScaleSize(l))
			continue;

		/* Check support format */
		if (!checkLayerFormat(l))
			continue;

		/* Check afbc align */
		if (!checkAFBCBlockSize(l))
			continue;

		/* Check rotation transform */
		if (!checkTransform(l))
			continue;

		l->setLayerAccelerator(ACCELERATOR_DISPC);
	}

	return 0;
}
