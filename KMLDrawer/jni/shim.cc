#include <jni.h>
#include <string.h>
#include <android/log.h>

#define DEBUG_TAG "NDK_MainActivity"

extern "C" {
	JNIEXPORT void JNICALL Java_org_falconview_kmldrawer_MainActivity_helloLog(JNIEnv* env, jobject obj, jstring logThis);
};

JNIEXPORT void JNICALL Java_org_falconview_kmldrawer_MainActivity_helloLog(JNIEnv* env, jobject obj, jstring logThis)
{
    jboolean isCopy;
    const char * szLogThis = env->GetStringUTFChars(logThis, &isCopy);
    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "NDK:LC: [%s]", szLogThis);
    env->ReleaseStringUTFChars(logThis, szLogThis);
}
