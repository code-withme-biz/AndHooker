#include <android/log.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdlib.h>
#include "log_util.h"
#include <jni.h>
#include "invoke_dex_method.h"

extern int invoke_dex_method(const char* dexPath, const char* dexOptDir, const char* className, const char* methodName, int argc, char *argv[]);

int hook_entry(char * apkPath) {
	LOGD("[++]Hook success, pid = %d\n", getpid());
	LOGD("[++]Injected %s\n", apkPath);

	char *cachePath = "/data/data/com.ss.android.ugc.aweme/files";
	char *className = "andhook/test/HookInit"; //"com/ss/android/ugc/aweme/splash/SplashActivity";
	char *methodName = "Inite_Hook";
    LOGD("[++]Start inject entry: %s, %s, %s, %s\n", apkPath, cachePath, className, methodName);
    int ret = invoke_dex_method(apkPath, cachePath, className, methodName, 0, NULL);
    LOGD("[++]APK inject result = %d\n", ret);
    return 0;
}

