/*
 * Copyright (C) 2010 The Android Open Source Project
 * Copyright (C) 2012-2015, The Linux Foundation. All rights reserved.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fcntl.h>
#include <hardware/hardware.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "SprdHWLayer.h"
#include "DpuDeviceAdf.h"

namespace android {

DpuDevice::DpuDevice():dev_id(0), intf_id(0), eng_id(0), intf(-1), eng(-1),
	mCapability(NULL), mDebugFlag(0), mDisable(0), mDpuCore(NULL) {
		memset(&dev, 0, sizeof(adf_device));
		memset(&engine_data, 0, sizeof(struct adf_overlay_engine_data));
	}

DpuDevice::~DpuDevice() {
	if (mDpuCore) {
		ALOGE("~DpuDevice delete mDpuCore\n");
		delete mDpuCore;
	}
}

int DpuDevice::prepare(SprdHWLayer **list, int count, bool *support) {
	char value[PROPERTY_VALUE_MAX];
	SprdHWLayer *l;

	if (list == NULL) {
		ALOGE("DpuDevice prepare list is NULL\n");
		return -1;
	}

	if (!mDpuCore) {
		ALOGE("Warning: init function must be call first");
		return -1;
	}

	mDebugFlag = mDpuCore->queryDebugFlag();

	property_get("debug.dpu.disable", value, "0");
	mDisable = atoi(value);

	if (mDisable ==  0) {
		mDpuCore->prepare(list, count, support, (int)mCapability->number_hwlayer);
	}

	ALOGD_IF(mDebugFlag, "[%s] support: %d", mDpuCore->getDpuVersion(),  *support);

	return 0;
}

int DpuDevice::init(bool *support) {
	int dev_id = 0;
	const __u32 fmt8888[] = { DRM_FORMAT_ARGB8888, };
	const size_t n_fmt8888 = sizeof(fmt8888) / sizeof(fmt8888[0]);

	int err = adf_device_open(dev_id, O_RDWR, &dev);
	if (err) {
		ALOGE("DpuDevice::init adf_device_open fail");
		return -1;
	}
	err = adf_find_simple_post_configuration(&dev, fmt8888, n_fmt8888, &intf_id,
						 &eng_id);
	if (err) {
		ALOGE("DpuDevice::init simple post configuration err");
		goto ERR0;
	}
	intf = adf_interface_open(&dev, intf_id, O_RDWR);
	if (intf < 0) {
		ALOGE("DpuDevice::init adf_interface_open err");
		goto ERR0;
	}
	eng = adf_overlay_engine_open(&dev, eng_id, O_RDWR);
	if (eng < 0) {
		ALOGE("DpuDevice::init adf_overlay_engine_open err");
		goto ERR0;
	}
	err = adf_get_overlay_engine_data(eng, &engine_data);
	if (err) {
		ALOGE("DpuDevice::init adf_get_overlay_engine_data err");
		goto ERR0;
	}

	struct adf_device_data data;
	err = adf_get_device_data(&dev, &data);
	if (err) {
		ALOGE("DpuDevice::init adf_get_device_data err");
		goto ERR0;
	}
	mDevCapability = (struct sprd_adf_device_capability*)(data.custom_data);
	ALOGD("Got dpu device_id = %d", mDevCapability->device_id);

	mDpuCore = DpuFactory::createDpuIpCore(mDevCapability->device_id);

	mCapability =(struct sprd_adf_overlayengine_capability *)(engine_data.custom_data);
	ALOGD("Got dpu hwlayer number: %d", mCapability->number_hwlayer);
	if (mCapability->number_hwlayer > 2)
		*support = true;
	else
		*support = false;
ERR0:
	adf_device_close(&dev);
	return 0;
}
} // namespace android
