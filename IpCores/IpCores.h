#ifndef _IPCORES_H_
#define _IPCORES_H_
#include "SprdHWLayer.h"
#include "SprdUtil.h"
#include "../debug_dpu.h"

// base class
class DpuIpCore {
public:
    DpuIpCore(const char *version): mDpuVersion(version) { }
    virtual ~DpuIpCore() { };
    virtual int prepare(SprdHWLayer **list, int count, bool *support, int dpu_limit);
    void updateDisplaySize(uint32_t w, uint32_t h);
    int queryDebugFlag();
    inline const char* getDpuVersion() {
        return mDpuVersion;
    }
    void setCornerSize(int cornerSize) {
	mCornerSize = cornerSize;
    }

protected:
    bool checkRGBLayerFormat(SprdHWLayer *l);
    bool checkAFBCBlockSize(SprdHWLayer *l);

    void dumpLayers(SprdHWLayer **list, int count);
    int mDebugFlag;
    const char *mDpuVersion;
    uint32_t mDispWidth;
    uint32_t mDispHeight;
    int mCornerSize;
};

class DpuR2P0 : public DpuIpCore {
public:
    DpuR2P0(const char *version): DpuIpCore(version) { }
    ~DpuR2P0() {}
    void cleanAccelerator(SprdHWLayer **list, int count);
    int prepare(SprdHWLayer **list, int count, bool *support, int dpu_limit);

protected:
    bool checkAFBCBlockSize(SprdHWLayer *l);
    bool checkBlendMode(SprdHWLayer *l);
    bool checkTransform(SprdHWLayer *l);
    bool checkLayerFormat(SprdHWLayer *l);
    bool checkScaleSize(SprdHWLayer *l);
    bool isMaxOverlayerOutOfLimit(SprdHWLayer **list, int count);
};

class DispcLiteR2P0 : public DpuIpCore {
public:
    DispcLiteR2P0(const char *version): DpuIpCore(version) { }
    ~DispcLiteR2P0() {}
    int prepare(SprdHWLayer **list, int count, bool *support, int dpu_limit);
};

class DpuLiteR1P0 : public DpuIpCore {
public:
    DpuLiteR1P0(const char *version): DpuIpCore(version) { }
    ~DpuLiteR1P0() {}
    int prepare(SprdHWLayer **list, int count, bool *support, int dpu_limit);
};

class DpuR3P0: public DpuIpCore {
public:
    DpuR3P0(const char *version): DpuIpCore(version) { }
    ~DpuR3P0() {}
    int prepare(SprdHWLayer **list, int count, bool *support, int dpu_limit);
protected:
    bool checkAFBCBlockSize(SprdHWLayer *l);
    bool checkBlendMode(SprdHWLayer *l);
    bool checkTransform(SprdHWLayer *l);
    bool checkLayerFormat(SprdHWLayer *l);
    bool checkScaleSize(SprdHWLayer *l);
};

class DpuR4P0: public DpuIpCore {
public:
    DpuR4P0(const char *version): DpuIpCore(version) { }
    ~DpuR4P0() {}
    int prepare(SprdHWLayer **list, int count, bool *support, int dpu_limit);
protected:
    bool checkLayerSize(SprdHWLayer *l);
    bool checkAFBCBlockSize(SprdHWLayer *l);
    bool checkBlendMode(SprdHWLayer *l);
    bool checkTransform(SprdHWLayer *l);
    bool checkLayerFormat(SprdHWLayer *l);
    bool checkScaleSize(SprdHWLayer *l);
};


/*
 * 0 - 1: default DpuIpCore
 * 2: Dispc_lite_r2p0 - sharkl2
 * 3: Dpu_r2p0 -sharkl3
 * 4: Dpu_lite_r1p0 - pike2/sharkle
 * 5: Dpu_r4p0 - sharkl5pro
 * 6: Dpu_lite_r2p0 - sharkl5
 */

enum ENUM_IPCORE{
    IPCORE_DEFAULT0,
    IPCORE_DEFAULT1,
    IPCORE_DISPC_LITE_R2P0,
    IPCORE_DPU_LITE_R1P0,
    IPCORE_DPU_R2P0,
    IPCORE_DPU_R3P0,
    IPCORE_DPU_R4P0,
    IPCORE_MAX_NUM,
};

static const char *dpu_cores[] = {
    "dpu-core",
    "dpu-core",
    "dispc-lite-r2p0",
    "dpu-lite-r1p0",
    "dpu-r2p0",
    "dpu-r3p0",
    "dpu-r4p0",
    "Unknown"
};

class DpuFactory {
public:
    static inline const char * getDpuVersionById(int device_id) {

        if (device_id < IPCORE_MAX_NUM)
            return dpu_cores[device_id];
        else
            return "Unknown";
    }

    static DpuIpCore *createDpuIpCore(int device_id) {
            return createDpuIpCore(getDpuVersionById(device_id));
    }

    static int compare(const char *version, const char *ip_version) {
        size_t sz = std::min(strlen(version), strlen(ip_version));
        return strncmp(version, ip_version, sz);
    }

    static DpuIpCore *createDpuIpCore(const char *dpu_version) {
        int device_id = -1;

        ALOGD("dpu_version: %s (%zu)", dpu_version, strlen(dpu_version));
        for (int i = 0; i < IPCORE_MAX_NUM; i++) {
            if (!compare(dpu_version, getDpuVersionById(i))) {
                ALOGD("Found dpu core: %d (%s)\n", i, getDpuVersionById(i));
                device_id = i;
                break;
            }
        }

        switch (device_id) {
            case IPCORE_DEFAULT0:
            case IPCORE_DEFAULT1:
                return new DispcLiteR2P0(dpu_version);
            case IPCORE_DISPC_LITE_R2P0:
                return new DispcLiteR2P0(dpu_version);
            case IPCORE_DPU_LITE_R1P0:
		return new DpuLiteR1P0(dpu_version);
            case IPCORE_DPU_R2P0:
                return new DpuR2P0(dpu_version);
            case IPCORE_DPU_R3P0:
                return new DpuR3P0(dpu_version);
            case IPCORE_DPU_R4P0:
                return new DpuR4P0(dpu_version);
            default:
                ALOGD("Get Unknown dpu version: %s", dpu_version);
                return nullptr;
        }

        return nullptr;
    }

};

#endif
