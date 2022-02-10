#include "IpCores.h"

// @Override
int DpuLiteR1P0::prepare(SprdHWLayer **list, int count, bool *support, int dpu_limit)
{
	HWC_IGNORE(support);
	SprdHWLayer *l;
	bool alpha_issue = false;
	bool premult_issue = true;

	if (count > dpu_limit) {
		ALOGW("Warning: Dispc cannot support %d layer blending(%d)", count, *support);
		return -1;
	}

	for (int i = 0; i < count; i++) {
		l = list[i];

		if (l->getBlendMode() != SPRD_HWC_BLENDING_PREMULT)
                        premult_issue = false;

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

	/* When the 360 panoramic camera takes a photo and
	 * adds a special effect, a black screen appears.
	 * Because in the premult mode, when the format is
	 * RGBA888, alpha is equal to 0, but RGB is not
	 * equal to 0.
	 * The problem scene is that the DPU processes
	 * the 3-layer PREMULT data and adjust the volume
	 * in this scene to become 4 layers of premult data.
	 * In order to workaround this problem, increase
	 * the judgment, DPU does not handle such scenes.*/
	if (alpha_issue || (premult_issue && count > 2)) {
		for (int i = 0; i < count; i++) {
			l = list[i];
			l->setLayerAccelerator(ACCELERATOR_NON);
		}
	}
	return 0;
}
