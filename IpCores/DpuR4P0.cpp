#include "IpCores.h"

#define __CLASS__ "DpuR4P0"
#define DPU_R4P0_MAX_PITCH	2400
#define DPU_R4P0_MAX_HEIGHT	4095

// @Override
bool DpuR4P0::checkLayerSize(SprdHWLayer *l)
{
	if (l == NULL)
		return false;

	const native_handle_t *pNativeHandle = l->getBufferHandle();

	/* DPU r4p0 not support over 4095 height*/
	if ((l->getSprdSRCRect()->y + l->getSprdSRCRect()->h) > DPU_R4P0_MAX_HEIGHT)
		return false;

	/* DPU r4p0 not support over 2400 stride. */
	if (ADP_STRIDE(pNativeHandle) > DPU_R4P0_MAX_PITCH)
		return false;

	return true;
}

// @Override
bool DpuR4P0::checkTransform(SprdHWLayer *l)
{
	if (l == NULL)
		return false;

	const native_handle_t *pNativeHandle = l->getBufferHandle();

	LOGI_IF(mDebugFlag, "checkTransform: Transform(%d), SRCRect(%d,%d)",
		l->getTransform(), l->getSprdSRCRect()->x, l->getSprdSRCRect()->y);

	if (l->getTransform() != 0) {
		/* DPU not support yuv422,rgb565,afbc-rgb565 rotation. */
		switch(ADP_FORMAT(pNativeHandle)) {
			case HAL_PIXEL_FORMAT_RGBA_8888:
			case HAL_PIXEL_FORMAT_RGBX_8888:
			case HAL_PIXEL_FORMAT_BGRA_8888:
				if (ADP_COMPRESSED(pNativeHandle)) {
					if (((l->getSprdSRCRect()->x % 16) != 0) ||
						((l->getSprdSRCRect()->y % 16) != 0)) {
						return false;
					}else
						break;
				}else {
					if (((l->getSprdSRCRect()->x % 2) != 0) ||
						((l->getSprdSRCRect()->y % 2) != 0)) {
						return false;
					}else
						break;
				}
			case HAL_PIXEL_FORMAT_YCbCr_420_SP:
			case HAL_PIXEL_FORMAT_YCrCb_420_SP:
			case HAL_PIXEL_FORMAT_YCbCr_420_888:
			case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
				if (ADP_COMPRESSED(pNativeHandle)) {
					if (((l->getSprdSRCRect()->x % 32) != 0) ||
						((l->getSprdSRCRect()->y % 8) != 0)) {
						return false;
					}else
						break;
				}else {
					if (((l->getSprdSRCRect()->x % 2) != 0) ||
						((l->getSprdSRCRect()->y % 2) != 0)) {
						return false;
					}else
						break;
				}
			case HAL_PIXEL_FORMAT_YV12:
				break;
			default:
				return false;
		}
	}

	return true;
}

// @Override
bool DpuR4P0::checkAFBCBlockSize(SprdHWLayer *l)
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
			/* RGB888 & RGB565 afbc align 16x16 */
			case HAL_PIXEL_FORMAT_RGBA_8888:
			case HAL_PIXEL_FORMAT_RGBX_8888:
			case HAL_PIXEL_FORMAT_BGRA_8888:
			case HAL_PIXEL_FORMAT_RGB_565:
				if (((ADP_STRIDE(pNativeHandle) % 16) == 0) &&
					((ADP_VSTRIDE(pNativeHandle) % 16) == 0))
					return true;
				break;
			/* YUV420 afbc align 32*8 */
			case HAL_PIXEL_FORMAT_YCbCr_420_SP:
			case HAL_PIXEL_FORMAT_YCrCb_420_SP:
			case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
				if (((ADP_STRIDE(pNativeHandle) % 32) == 0) &&
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
bool DpuR4P0::checkLayerFormat(SprdHWLayer *l)
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
		case HAL_PIXEL_FORMAT_YCbCr_420_888:
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
bool DpuR4P0::checkScaleSize(SprdHWLayer *l)
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
int DpuR4P0::prepare(SprdHWLayer **list, int count, bool *support, int dpu_limit)
{
	HWC_IGNORE(support);
	SprdHWLayer *l;
	int adapted_num = 0;
	int correct_limit = 0;

	if (count > dpu_limit)
		correct_limit = dpu_limit - 1;
	else
		correct_limit = dpu_limit;

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

		if (!checkLayerSize(l))
			continue;
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
