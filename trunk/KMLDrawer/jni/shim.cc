#include <jni.h>
#include <string.h>
#include <android/log.h>
#include <sstream>

#define DEBUG_TAG "NDK_MainActivity"

extern "C" {
	JNIEXPORT void JNICALL Java_org_falconview_kmldrawer_MainActivity_helloLog(JNIEnv* env, jobject obj, jstring logThis);
};

extern int run_tests_main(int argc, char **argv);
bool tests_run = false;

JNIEXPORT void JNICALL Java_org_falconview_kmldrawer_MainActivity_helloLog(JNIEnv* env, jobject obj, jstring logThis)
{
    jboolean isCopy;
    const char * szLogThis = env->GetStringUTFChars(logThis, &isCopy);
    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "NDK:LC: [%s]", szLogThis);
    env->ReleaseStringUTFChars(logThis, szLogThis);

    if (!tests_run)
    {
    	char* argv[1] = {(char*)"foo"};
    	int argc = 1;
    	int rv = run_tests_main(argc, argv);
    	tests_run = true;

    	std::stringstream ss;
    	ss << "Tests returned: " << rv;

    	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, ss.str().c_str());
    }
}
