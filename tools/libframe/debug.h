#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>
#include <utils/Log.h>

#define LOGD_IF(cond, format, ...)    ALOGD_IF(cond, __CLASS__ "::%s <%d>: " format, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOGE_IF(cond, format, ...)    ALOGE_IF(cond, __CLASS__ "::%s <%d>: " format, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOGI_IF(cond, format, ...)    ALOGI_IF(cond, __CLASS__ "::%s <%d>: " format, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#if 0
#define LOGD(format, ...)    LOGD_IF(1, format, ##__VA_ARGS__)
#define LOGE(format, ...)    LOGE_IF(1, format, ##__VA_ARGS__)
#define LOGI(format, ...)    LOGI_IF(1, format, ##__VA_ARGS__)
#else
#define LOGD(format, ...)    fprintf(stderr, __CLASS__ "::%-28s <%d>: " format "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOGE(format, ...)    fprintf(stderr, __CLASS__ "::%-28s <%d>: " format "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define LOGI(format, ...)    fprintf(stderr, __CLASS__ "::%-28s <%d>: " format "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#define XDBG(x)  LOGD(#x" = 0x%x", x)
#define DDBG(x)  LOGD(#x" = %d", x)
#define PDBG(x)  LOGD(#x" = %p", x)
#define SDBG(x) LOGD(#x" = %s", x)

#define HWC_ASSERT(x) do {\
	LOGE(#x "is invalid"); \
	return -ERR_BAD_PARAMETER; \
	}while (0)

#ifndef ALIGN
#define ALIGN(value, base) (((value) + ((base)-1)) & ~((base)-1))
#endif

#endif