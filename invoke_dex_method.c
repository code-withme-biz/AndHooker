#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <jni.h>
#include <dlfcn.h>
#include "log_util.h"

JNIEnv* (*getJNIEnv)();

JavaVM* g_jvm;
JNIEnv* g_env;

jboolean g_attach_flag;
/**
 * DEX화일을 인젝트하고 메소드를 실행한다.
 * @param dexPath 인젝트하는 dex화일
 * @param dexOptDir 캐쉬경로. (타겟앱, 타겟프로세스의 쓰기권한에 주의할것)
 * @param className 인젝트한 다음에 실행되는 클라스명
 * @param methodName 실행하려는 메소드명
 * @param argc 파라메터 개수
 * @param argv 파라메터
 * @return
 */


int invoke_dex_method(const char* dexPath, const char* dexOptDir, const char* className, const char* methodName, int argc, char *argv[]) {
     LOGD("[+++]dexPath = %s, dexOptDir = %s, className = %s, methodName = %s\n", dexPath, dexOptDir, className, methodName);
    // JNIEnv 얻기
    g_attach_flag = JNI_FALSE;

	void* handle = dlopen("/system/lib/libandroid_runtime.so", RTLD_NOW);
    getJNIEnv = dlsym(handle, "_ZN7android14AndroidRuntime9getJNIEnvEv");

    JNIEnv* env = getJNIEnv();
	
	(*env)->GetJavaVM(env, &g_jvm);
	if( g_jvm == NULL)
	{
		LOGD("[+++]GetJavaVM: get jvm failed.\n");
		return -1;
	}

	int status = (*g_jvm)->GetEnv(g_jvm, (void **)&env, JNI_VERSION_1_6);
    if( status != JNI_OK )
	{
		LOGD("[+++]GetEnv: not maded\n");
		int getEnvStat = (*g_jvm)->AttachCurrentThread(g_jvm, (void**)&env, JNI_VERSION_1_6);
		if( getEnvStat != JNI_EDETACHED)
		{
			LOGD("[+++]GetEnv: not attached\n");
			return JNI_FALSE;
		}
		else if(getEnvStat == JNI_EVERSION)
		{
			LOGD("[+++]GetEnv: version not supported\n");
			return JNI_FALSE;
		}
		g_attach_flag = JNI_TRUE;
	}
	else
	{
		LOGD("[+++]GetEnv: Already made...\n");
	}
	
	LOGD("[+++]GetEnv: %x\n", (uint32_t)*env);

    // ClassLoader의 getSystemClassLoader를 호출하고 현재프로세스의 ClassLoader를 얻는다.
    jclass classloaderClass = (*env)->FindClass(env,"java/lang/ClassLoader");
    LOGD("[+++]classloaderClass = %x\n", (uint32_t)classloaderClass);
    jmethodID getsysloaderMethod = (*env)->GetStaticMethodID(env,classloaderClass, "getSystemClassLoader", "()Ljava/lang/ClassLoader;");
    LOGD("[+++]getsysloaderMethod = %x\n", (uint32_t)getsysloaderMethod);
    jobject loader = (*env)->CallStaticObjectMethod(env, classloaderClass, getsysloaderMethod);
    LOGD("[+++]loader = %x\n", (uint32_t)loader);

    // 현재ClassLoader로 처리하기 위해 DexClassLoader로 dex화일을 로드한다.
    jstring dexpath = (*env)->NewStringUTF(env, dexPath);
	//LOGD("dexpath = %s\n", (uint8_t*)dexpath);
    jstring dex_odex_path = (*env)->NewStringUTF(env,dexOptDir);
	//LOGD("dex_odex_path = %s\n", (uint8_t*)dex_odex_path);
    jclass dexLoaderClass = (*env)->FindClass(env,"dalvik/system/DexClassLoader");
    LOGD("[+++]dexLoaderClass = %x\n", (uint32_t)dexLoaderClass);
    jmethodID initDexLoaderMethod = (*env)->GetMethodID(env, dexLoaderClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V");
    LOGD("[+++]initDexLoaderMethod = %x\n", (uint32_t)initDexLoaderMethod);
    jobject dexLoader = (*env)->NewObject(env, dexLoaderClass, initDexLoaderMethod,dexpath,dex_odex_path,NULL,loader);
    LOGD("[+++]dexLoader = %x\n", (uint32_t)dexLoader);

    // DexClassLoader를 사용해서 실행하려는 코드를 로드한다.
    jmethodID findclassMethod = (*env)->GetMethodID(env,dexLoaderClass,"findClass","(Ljava/lang/String;)Ljava/lang/Class;");
    LOGD("[+++]findclassMethod = %x\n", (uint32_t)findclassMethod);
    jstring javaClassName = (*env)->NewStringUTF(env,className);
	//LOGD("javaClassName = %s\n", (uint8_t *)javaClassName);
    jclass javaClientClass = (*env)->CallObjectMethod(env,dexLoader,findclassMethod,javaClassName);
    if (!javaClientClass) {
        LOGD("Failed to load target class %s\n", className);
    }

	// 인젝트하려는 메소드를 로드한다.
    jmethodID start_inject_method = (*env)->GetStaticMethodID(env, javaClientClass, methodName, "()V");
    if (!start_inject_method) {
        LOGD("[+++]Failed to load target method %s\n", methodName);
        return -1;
    }
    // 메소드실행 (이 메소드는 public static void여야 한다.)
    (*env)->CallStaticVoidMethod(env,javaClientClass,start_inject_method);
	
	if(g_attach_flag)
	{
		(*g_jvm)->DetachCurrentThread(g_jvm);
		LOGD("[+++]GetEnv: detached..\n");
		g_attach_flag = JNI_FALSE;
	}
    //return JNI_TRUE;
    return 0;
}
