#ifndef _GVFS_NATIVE_H_
#define _GVFS_NATIVE_H_

#include <gglobal.h>
#include <gstdio.h>

#ifdef __cplusplus
extern "C" {
#endif

G_API void gvfs_init();
G_API void gvfs_cleanup();

G_API void gvfs_setCodeKey(const char key[16]);
G_API void gvfs_setAssetsKey(const char key[16]);

#ifdef __cplusplus
}
#endif

#endif
