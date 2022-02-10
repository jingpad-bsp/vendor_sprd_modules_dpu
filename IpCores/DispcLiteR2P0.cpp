#include "IpCores.h"

// @Override
int DispcLiteR2P0::prepare(SprdHWLayer **list, int count, bool *support, int dpu_limit)
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
