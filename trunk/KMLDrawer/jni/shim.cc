#include <jni.h>
#include <string.h>
#include <android/log.h>
#include <sstream>

#include "shim.h"

#define DEBUG_TAG "NDK_MainActivity"

extern "C"
{
	JNIEXPORT void JNICALL Java_org_falconview_kmldrawer_MainActivity_nativeLog(
			JNIEnv* env, jobject obj, jstring logThis);
	JNIEXPORT void JNICALL Java_org_falconview_kmldrawer_MainActivity_initDrawer(
			JNIEnv* env, jobject obj, jstring kml_url);
	JNIEXPORT void JNICALL Java_org_falconview_kmldrawer_MainActivity_drawKML(
			JNIEnv* env, jobject obj, jint height, jint width);
};

extern int run_tests_main(int argc, char **argv);
bool tests_run = false;

JNIEXPORT void JNICALL Java_org_falconview_kmldrawer_MainActivity_nativeLog(
		JNIEnv* env, jobject obj, jstring logThis)
{
	// log
    jboolean isCopy;
    const char * szLogThis = env->GetStringUTFChars(logThis, &isCopy);
    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "NDK:LC: [%s]", szLogThis);
    env->ReleaseStringUTFChars(logThis, szLogThis);

    // run tests on first call
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

////
//// OBJECTS AND FUNCTIONS FOR DRAWING
////

AndroidShim* the_shim = nullptr;
kmldrawing::KMLDrawer* the_drawer = nullptr;

JNIEXPORT void JNICALL Java_org_falconview_kmldrawer_MainActivity_initDrawer(
		JNIEnv* env, jobject obj, jstring kml_url)
{
    __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Initializing drawer...");

	// instantiate the objects
	the_shim = new AndroidShim;
	the_drawer = new kmldrawing::KMLDrawer(the_shim);

	// open the kml
    jboolean is_copy;
    const char* url = env->GetStringUTFChars(kml_url, &is_copy);
	if (the_drawer->OpenKML(url))
		__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Open SUCCESS");
	else
		__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, "Open FAILED");
    env->ReleaseStringUTFChars(kml_url, url);
}

JNIEXPORT void JNICALL Java_org_falconview_kmldrawer_MainActivity_drawKML(
		JNIEnv* env, jobject obj, jint height, jint width)
{
	the_shim->m_height = height;
	the_shim->m_width = width;
	the_shim->m_env = env;
	the_shim->m_obj = obj;

	the_drawer->Draw();

	the_shim->CleanupAfterDraw();
}

////
//// SHIM METHODS
////

void AndroidShim::BeginDraw()
{
	//LogMessage("NOT IMPLEMENTED: BeginDraw");
}

void AndroidShim::EndDraw()
{
	//LogMessage("NOT IMPLEMENTED: EndDraw");
}

void AndroidShim::BeginDrawElement(const kmldom::ElementPtr& element)
{
	//LogMessage("NOT IMPLEMENTED: BeginDrawElement");
}

void AndroidShim::EndDrawElement(const kmldom::ElementPtr& element)
{
	//LogMessage("NOT IMPLEMENTED: EndDrawElement");
}

void AndroidShim::GetSurfaceSize(int* width, int* height)
{
	*width = m_width;
	*height = m_height;

//	std::stringstream ss;
//	ss << "GetSurfaceSize Height: " << m_height;
//	LogMessage(ss.str().c_str());
}

void AndroidShim::GetMapBounds(double* bottom_lat, double* left_lon,
      double* top_lat, double* right_lon)
{
	//LogMessage("NOT IMPLEMENTED: GetMapBounds");
	*bottom_lat = 23.0;
	*left_lon = -127.0;
	*top_lat = 50.0;
	*right_lon = -67.0;
}

void AndroidShim::GetDegreesPerPixel(double* dpp_y, double* dpp_x)
{
	//LogMessage("NOT IMPLEMENTED: GetDegreesPerPixel");
	double bottom_lat, left_lon, top_lat, right_lon;
	GetMapBounds(&bottom_lat, &left_lon, &top_lat, &right_lon);
	*dpp_y = (top_lat - bottom_lat) / m_height;
	*dpp_x = (right_lon - left_lon) / m_width;
}

void AndroidShim::GeoToSurface(double lat, double lon, int* x, int* y)
{
	//LogMessage("NOT IMPLEMENTED: GeoToSurface");
	double dpp_y, dpp_x;
	double bottom_lat, left_lon, top_lat, right_lon;
	GetDegreesPerPixel(&dpp_y, &dpp_x);
	GetMapBounds(&bottom_lat, &left_lon, &top_lat, &right_lon);
	*x = (lon - left_lon)/dpp_x;
	*y = m_height - (lat - bottom_lat)/dpp_y;

//	std::stringstream ss;
//	ss << "right_lon: " << right_lon;
//	ss << " lon: " << lon;
//	ss << " dpp_x: " << dpp_x;
//	LogMessage(ss.str());
}

void AndroidShim::SurfaceToGeo(int x, int y, double* lat, double* lon)
{
	LogMessage("NOT IMPLEMENTED: SurfaceToGeo");
}

void AndroidShim::CreatePen(
      const kmlbase::Color32& color, float width, kmldrawing::Pen** pen)
{
	//LogMessage("NOT IMPLEMENTED: CreatePen");
	*pen = new AndroidPen;
	m_pens_to_delete_after_each_draw.push(*pen);
}

void AndroidShim::CreateBrush(
      const kmlbase::Color32& color, kmldrawing::Brush** brush)
{
	//LogMessage("NOT IMPLEMENTED: CreateBrush");
	*brush = new AndroidBrush;
	m_brushes_to_delete_after_each_draw.push(*brush);
}

void AndroidShim::CreateImageFromRawBytes(
      unsigned char* bytes, size_t length, kmldrawing::Image** image,
      bool will_be_used_for_geo /*= false*/)
{
	LogMessage("NOT IMPLEMENTED: CreateImageFromRawBytes");
}

void AndroidShim::DrawImage(const kmldrawing::Image& image,
      float left, float top, float right, float bottom, float rotation)
{
	LogMessage("NOT IMPLEMENTED: DrawImage");
}

void AndroidShim::DrawImageGeo(const kmldrawing::Image& image,
   double west, double north, double east, double south)
{
	LogMessage("NOT IMPLEMENTED: DrawImageGeo");
}

void AndroidShim::DrawEllipse(float x, float y, float x_axis, float y_axis,
      const kmldrawing::Pen& pen, const kmldrawing::Brush& brush)
{
	//LogMessage("NOT IMPLEMENTED: DrawEllipse");

	jclass cls = m_env->GetObjectClass(m_obj);
	jmethodID mid = m_env->GetMethodID(cls, "addEllipse", "(FF)V");

	if (mid == 0)
	{
		LogMessage("addEllipse not found");
		return;
	}

	m_env->CallVoidMethod(m_obj, mid, x, y);
}

void AndroidShim::DrawPolyPolygon(
      const std::vector<std::pair<float, float>>& points,
      const std::vector<int>& point_counts,
      const kmldrawing::Pen& pen, const kmldrawing::Brush& brush)
{
	//LogMessage("NOT IMPLEMENTED: DrawPolyPolygon");

	jclass cls = m_env->GetObjectClass(m_obj);
	jmethodID mid = m_env->GetMethodID(cls, "addLine", "(FFFF)V");

	if (mid == 0)
	{
		LogMessage("addLine not found");
		return;
	}

	// TODO: inner rings

	size_t point_count = point_counts[0];
	for (size_t i = 0; i < point_count - 1; i++)
		m_env->CallVoidMethod(
				m_obj, mid, points[i].first, points[i].second, points[i + 1].first, points[i + 1].second);
	// close the polygon
	m_env->CallVoidMethod(m_obj, mid, points[point_count - 1].first, points[point_count - 1].second,
			points[0].first, points[0].second);
}

void AndroidShim::DrawLines(const std::vector<std::pair<float, float>>& points,
      const kmldrawing::Pen& pen)
{
	LogMessage("NOT IMPLEMENTED: DrawLines");
}

void AndroidShim::DrawString(const std::string& text, float x, float y,
   float size, const kmldrawing::Pen& pen)
{
	LogMessage("NOT IMPLEMENTED: DrawString");
}

void AndroidShim::ShowInformation(
      const std::string& title, const std::string& text)
{
	LogMessage("NOT IMPLEMENTED: ShowInformation");
}

void AndroidShim::SetTimer(int id, unsigned long ms)
{
	LogMessage("NOT IMPLEMENTED: SetTimer");
}

void AndroidShim::ClearAllTimers()
{
	//LogMessage("NOT IMPLEMENTED: ClearAllTimers");
}

void AndroidShim::Invalidate(void* ptr)
{
	LogMessage("NOT IMPLEMENTED: Invalidate");
}

extern std::string TEST_COORDINATES_KML_DATA;
extern std::string US_STATES_KML;

bool AndroidShim::Fetch(const std::string& uri, std::string* data)
{
   //*data = "<kml><Placemark><Point><coordinates>-84,34</coordinates></Point></Placemark></kml>";
   *data = US_STATES_KML;
   return true;
}

bool AndroidShim::ShouldRender(const kmldom::TimePrimitivePtr& time_primitive)
{
   return true;
}

void AndroidShim::LogMessage(const std::string& message)
{
	__android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, message.c_str());
}

void AndroidShim::CleanupAfterDraw()
{
   while (m_pens_to_delete_after_each_draw.size() > 0)
   {
      delete m_pens_to_delete_after_each_draw.top();
      m_pens_to_delete_after_each_draw.pop();
   }

   while (m_brushes_to_delete_after_each_draw.size() > 0)
   {
      delete m_brushes_to_delete_after_each_draw.top();
      m_brushes_to_delete_after_each_draw.pop();
   }
}
