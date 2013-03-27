#include <jni.h>
#include <string.h>
#include <android/log.h>
#include <sstream>
#include "shim.h"

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

void AndroidShim::BeginDraw() {}
void AndroidShim::EndDraw() {}
void AndroidShim::BeginDrawElement(const kmldom::ElementPtr& element) {}
void AndroidShim::EndDrawElement(const kmldom::ElementPtr& element) {}
void AndroidShim::GetSurfaceSize(int* width, int* height) {}
void AndroidShim::GetMapBounds(double* bottom_lat, double* left_lon,
      double* top_lat, double* right_lon) {}
void AndroidShim::GetDegreesPerPixel(double* dpp_y, double* dpp_x) {}
void AndroidShim::GeoToSurface(double lat, double lon, int* x, int* y) {}
void AndroidShim::SurfaceToGeo(int x, int y, double* lat, double* lon) {}
void AndroidShim::CreatePen(
      const kmlbase::Color32& color, float width, kmldrawing::Pen** pen) {}
void AndroidShim::CreateBrush(
      const kmlbase::Color32& color, kmldrawing::Brush** brush) {}

void AndroidShim::CreateImageFromRawBytes(
      unsigned char* bytes, size_t length, kmldrawing::Image** image,
      bool will_be_used_for_geo /*= false*/)
{
}

void AndroidShim::DrawImage(const kmldrawing::Image& image,
      float left, float top, float right, float bottom, float rotation)
{
}

void AndroidShim::DrawImageGeo(const kmldrawing::Image& image,
   double west, double north, double east, double south)
{
}

void AndroidShim::DrawEllipse(float x, float y, float x_axis, float y_axis,
      const kmldrawing::Pen& pen, const kmldrawing::Brush& brush) {}
void AndroidShim::DrawPolyPolygon(
      const std::vector<std::pair<float, float>>& points,
      const std::vector<int>& point_counts,
      const kmldrawing::Pen& pen, const kmldrawing::Brush& brush) {}
void AndroidShim::DrawLines(const std::vector<std::pair<float, float>>& points,
      const kmldrawing::Pen& pen) {}
void AndroidShim::DrawString(const std::string& text, float x, float y,
   float size, const kmldrawing::Pen& pen) {}
void AndroidShim::ShowInformation(
      const std::string& title, const std::string& text) {}

void AndroidShim::SetTimer(int id, unsigned long ms)
{
}

void AndroidShim::ClearAllTimers() {}
void AndroidShim::Invalidate(void* ptr) {}

bool AndroidShim::Fetch(const std::string& uri, std::string* data)
{
   return false;
}

bool AndroidShim::ShouldRender(const kmldom::TimePrimitivePtr& time_primitive)
{
   return true;
}

