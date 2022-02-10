#include "IpCores.h"
#include <cutils/properties.h>

int DpuIpCore::queryDebugFlag() {
	char value[PROPERTY_VALUE_MAX];

	property_get("debug.dpu.info", value,  "0");

	mDebugFlag = atoi(value);
	return mDebugFlag;
}

void DpuIpCore::dumpLayers(SprdHWLayer **list, int count) {
	HWC_IGNORE(list);
	HWC_IGNORE(count);
}

void DpuIpCore::updateDisplaySize(uint32_t w, uint32_t h) {
        mDispWidth = w;
        mDispHeight = h;
}

bool DpuIpCore::checkRGBLayerFormat(SprdHWLayer *l) {
    if (l == NULL)
    {
        return false;
    }

    const native_handle_t *pNativeHandle = l->getBufferHandle();

    if (pNativeHandle == NULL)
    {
        return false;
    }

    switch (ADP_FORMAT(pNativeHandle)) {
	case HAL_PIXEL_FORMAT_RGBA_8888:
	case HAL_PIXEL_FORMAT_RGBX_8888:
	case HAL_PIXEL_FORMAT_RGB_888:
	case HAL_PIXEL_FORMAT_RGB_565:
	case HAL_PIXEL_FORMAT_BGRA_8888:
		return true;
	default:
	    break;
	}
    return false;
}

bool DpuIpCore::checkAFBCBlockSize(SprdHWLayer *l) {
  if (l == NULL)
    return false;

  const native_handle_t *pNativeHandle = l->getBufferHandle();

  if (pNativeHandle == NULL) {
    return false;
  }

  if (ADP_COMPRESSED(pNativeHandle)) {
    if (ADP_FORMAT(pNativeHandle) == HAL_PIXEL_FORMAT_RGB_565)
      return false;
    if (((ADP_STRIDE(pNativeHandle) % 16) == 0) &&
        ((ADP_VSTRIDE(pNativeHandle) % 16) == 0)) {
      return true;
    } else {
      ALOGW("Dpu checked invalid handle size under AFBC: pitch: %d, vstride:%d",
            ADP_STRIDE(pNativeHandle), ADP_VSTRIDE(pNativeHandle));
      return false;
    }
  }
  return true;
}

int DpuIpCore::prepare(SprdHWLayer **list, int count, bool *support, int dpu_limit)
{
	HWC_IGNORE(support);
	SprdHWLayer *l;
	bool alpha_issue = false;

	if (count > dpu_limit) {
		ALOGW("Warning: Dispc cannot support %d layer blending(%d)", count, *support);
		return -1;
	}

	for (int i = 0; i < count; i++) {
		l = list[i];

		/*sharkl2 dpu cannot handle alpha plane*/
		if (l->getPlaneAlpha() != 0xff) {
			alpha_issue = true;
			break;
		}
		if (!checkRGBLayerFormat(l))
			continue;
		if (l->getTransform() != 0)
			continue;
		if ((l->getSprdSRCRect()->w != l->getSprdFBRect()->w)
				|| (l->getSprdSRCRect()->h != l->getSprdFBRect()->h))
			continue;
		if (!checkAFBCBlockSize(l))
			continue;
		l->setLayerAccelerator(ACCELERATOR_DISPC);
	}

	if (alpha_issue) {
		for (int i = 0; i < count; i++) {
			l = list[i];
			l->setLayerAccelerator(ACCELERATOR_NON);
		}
	}

	return 0;
}
