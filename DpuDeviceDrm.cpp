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

#define DRM_DEVICE "/dev/dri/card0"

#include <fcntl.h>
#include <hardware/hardware.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "SprdHWLayer.h"
#include "DpuDeviceDrm.h"
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm/drm_mode.h>

namespace android {

DpuDevice::DpuDevice()
    : mDebugFlag(0), mDisable(0), mDpuCore(NULL), mMaxLayers(0) {}

DpuDevice::~DpuDevice() {
  if (mDpuCore) {
    ALOGE("~DpuDevice delete mDpuCore\n");
    delete mDpuCore;
  }
}

int64_t DpuDevice::GetProperty(uint32_t obj_id, uint32_t obj_type,
                               const char *prop_name, int fd) {
  drmModeObjectPropertiesPtr props;
  int64_t value;

  props = drmModeObjectGetProperties(fd, obj_id, obj_type);
  if (!props) {
    ALOGE("Failed to get properties for %d/%x", obj_id, obj_type);
    return -ENODEV;
  }

  bool found = false;
  for (int i = 0; !found && (size_t)i < props->count_props; ++i) {
    drmModePropertyPtr p = drmModeGetProperty(fd, props->props[i]);
    if (!strcmp(p->name, prop_name)) {
      value = props->prop_values[i];
      found = true;
    }
    drmModeFreeProperty(p);
  }

  drmModeFreeObjectProperties(props);
  return found ? value : -ENOENT;
}

int DpuDevice::prepare(SprdHWLayer **list, int count, bool *support) {
  char value[PROPERTY_VALUE_MAX];
  SprdHWLayer *l;
  int dpu_limit = mMaxLayers;

  if (list == NULL) {
    ALOGE("DpuDevice prepare list is NULL\n");
    return -1;
  }

  // WORKAROUND for wechat video call has 1 invalid layer(640x480->1x1).
  // FIXME: If wechat fix this invalid layer,this patch will be reverted.

  for (int i = 0; i < count; i++) {
    l = list[i];
    if (l == NULL) {
      ALOGE("DpuDevice prepare list[%d] is NULL\n",i);
      return -1;
    }

    if ((l->getSprdFBRect()->w == 1) && (l->getSprdFBRect()->h == 1) &&
       (l->getSprdSRCRect()->w == 640) && (l->getSprdSRCRect()->h == 480)) {
      l->setLayerAccelerator(ACCELERATOR_DISPC);
    }
  }

  if (!mDpuCore) {
    ALOGE("Warning: init function must be call first");
    return -1;
  }

  mDebugFlag = mDpuCore->queryDebugFlag();

  property_get("debug.dpu.disable", value, "0");
  mDisable = atoi(value);

  if (mDisable == 0) {
    if (mDpuCore->prepare(list, count, support, dpu_limit)) return -1;
  }

  ALOGD_IF(mDebugFlag, "[%s] support: %d", mDpuCore->getDpuVersion(), *support);

  return 0;
}

int DpuDevice::init(bool *support) {
  int fd;
  int64_t version_blob_id;
  char dpu_version[64];
  int32_t corner_size;

  fd = open(DRM_DEVICE, O_RDWR, S_IRWXU);
  if (fd < 0) {
    ALOGE("open drm device failed fd=%d.", fd);
    return -1;
  }

  int err = drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
  if (err) {
    ALOGE("dpu failed to set universal plane cap: %d", err);
    close(fd);
    return -1;
  }

  struct drm_mode_get_plane_res plane_res;
  memset(&plane_res, 0, sizeof(plane_res));
  err = drmIoctl(fd, DRM_IOCTL_MODE_GETPLANERESOURCES, &plane_res);
  if (err) {
    ALOGE("dpu failed to get plane resources: %d", err);
    close(fd);
    return -1;
  }

  mMaxLayers = plane_res.count_planes;
  ALOGI("dpu max layers:%d", mMaxLayers);

  drmModeResPtr res = drmModeGetResources(fd);
  if (!res) {
    ALOGE("Failed to get DrmResources resources");
    close(fd);
    return -1;
  }

  drmModeCrtcPtr c = drmModeGetCrtc(fd, res->crtcs[0]);
  if (!c) {
    ALOGE("Failed to get crtc %d", res->crtcs[0]);
    close(fd);
    drmModeFreeResources(res);
    return -1;
  }

  drmModeFreeResources(res);

  version_blob_id =
      GetProperty(c->crtc_id, DRM_MODE_OBJECT_CRTC, "dpu version", fd);
  if (version_blob_id < 0) {
    ALOGE("Failed to get dpu version property");
    close(fd);
    drmModeFreeCrtc(c);
    return -1;
  }

  drmModePropertyBlobPtr blob = drmModeGetPropertyBlob(fd, version_blob_id);
  if (!blob) {
    ALOGE("Failed to Get dpu version");
    close(fd);
    return -1;
  }

  memcpy(dpu_version, blob->data, blob->length);
  ALOGI("get dpu version: %s. length:%d", dpu_version, blob->length);

  corner_size =
      GetProperty(c->crtc_id, DRM_MODE_OBJECT_CRTC, "corner size", fd);
  if (corner_size < 0)
    ALOGW("Failed to get corner size property");
  else
    ALOGI("get corner size: %d.", corner_size);

  drmModeFreeCrtc(c);

  mDpuCore = DpuFactory::createDpuIpCore(dpu_version);
  if (mDpuCore) *support = true;
  close(fd);

  drmModeFreePropertyBlob(blob);

  if (mDpuCore)
    mDpuCore->setCornerSize(corner_size);

  return 0;
}

int DpuDevice::updateDisplaySize(uint32_t w, uint32_t h) {

  if (!mDpuCore) {
    ALOGE("Warning: init function must be call first");
    return -1;
  }

  mDpuCore->updateDisplaySize(w, h);

  return 0;
}
}  // namespace android
