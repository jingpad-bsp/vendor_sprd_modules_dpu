#include "IpCores.h"

#define __CLASS__ "DpuR3P0"
// @Override
bool DpuR3P0::checkTransform(SprdHWLayer *l)
{
	if (l == NULL)
		return false;

	const native_handle_t *pNativeHandle = l->getBufferHandle();

	LOGI_IF(mDebugFlag, "checkTransform: Transform(%d), SRCRect(%d,%d)",
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
bool DpuR3P0::checkAFBCBlockSize(SprdHWLayer *l)
{
	if (l == NULL)
		return false;

	const native_handle_t *pNativeHandle = l->getBufferHandle();

	if (pNativeHandle == NULL)
		return false;

	LOGI_IF(mDebugFlag, "checkAFBCBlockSize: compressed(%d),"
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

		LOGD("invalid handle size under AFBC: pitch: %d, vstride:%d",
			ADP_STRIDE(pNativeHandle), ADP_VSTRIDE(pNativeHandle));
		return false;
	}

	return true;
}

// @Override
bool DpuR3P0::checkLayerFormat(SprdHWLayer *l)
{
	if (l == NULL)
		return false;

	const native_handle_t *pNativeHandle = l->getBufferHandle();

	if (pNativeHandle == NULL)
		return false;

	LOGI_IF(mDebugFlag, "checkLayerFormat: Format(%d)",
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
bool DpuR3P0::checkScaleSize(SprdHWLayer *l)
{
	if (l == NULL)
		return false;

	uint32_t tform = l->getTransform();

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

// @Override
int DpuR3P0::prepare(SprdHWLayer **list, int count, bool *support, int dpu_limit)
{
	HWC_IGNORE(support);
	SprdHWLayer *l;
	int adapted_num = 0;
	int correct_limit = 0;

	if (count > dpu_limit)
		correct_limit = dpu_limit - 1;
	else
		correct_limit = dpu_limit;

	LOGI_IF(mDebugFlag, "prepare: count(%d)", count);

	for (int i = 0; i < count; i++) {
		l = list[i];
		if (l == NULL) {
			return -1;
		}

		if (l->getCompositionType() == COMPOSITION_SOLID_COLOR) {
			l->setLayerAccelerator(ACCELERATOR_DISPC);
			ALOGI_IF(mDebugFlag, "dpu dim layer");
			adapted_num++;
			if (correct_limit < adapted_num)
				l->setLayerAccelerator(ACCELERATOR_DISPC_BACKUP);
			continue;
		}

		/* Check support format */
		if (!checkLayerFormat(l))
			continue;
		/* Check rotation transform */
		if (!checkTransform(l))
			continue;
		if (!checkScaleSize(l))
			continue;
		if (!checkAFBCBlockSize(l))
			continue;
		l->setLayerAccelerator(ACCELERATOR_DISPC);
		adapted_num++;

		if (correct_limit < adapted_num)
			l->setLayerAccelerator(ACCELERATOR_DISPC_BACKUP);
	}

	return 0;
}
