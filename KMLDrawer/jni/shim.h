#ifndef KML_TEST_SHIM_H_
#define KML_TEST_SHIM_H_

#include "KMLDrawingLibrary.h"

class AndroidPen : public kmldrawing::Pen
{
};

class AndroidBrush : public kmldrawing::Brush
{
};

class AndroidImage : public kmldrawing::Image
{
public:
   AndroidImage(int index) : m_index(index) {}

   /*virtual*/ void GetSize(int* width, int* height) const
   {
      // TODO
      *width = 256;
      *height = 256;
   }

   int m_index;
};

class AndroidShim : public kmldrawing::KMLShim
{
public:
   AndroidShim() : m_ul_lat(50.0), m_ul_lon(-127.0), m_lr_lat(23.0), m_lr_lon(-67.0)
   {
   }

   void CleanupAfterDraw();

   virtual void BeginDraw();
   virtual void EndDraw();
   virtual void BeginDrawElement(const kmldom::ElementPtr& element);
   virtual void EndDrawElement(const kmldom::ElementPtr& element);
   virtual void GetSurfaceSize(int* width, int* height);
   virtual void GetMapBounds(double* bottom_lat, double* left_lon,
      double* top_lat, double* right_lon);
   virtual void GetDegreesPerPixel(double* dpp_y, double* dpp_x);
   virtual void GeoToSurface(double lat, double lon, int* x, int* y);
   virtual void SurfaceToGeo(int x, int y, double* lat, double* lon);
   virtual void CreatePen(
      const kmlbase::Color32& color, float width, kmldrawing::Pen** pen);
   virtual void CreateBrush(
      const kmlbase::Color32& color, kmldrawing::Brush** brush);
   virtual void CreateImageFromRawBytes(
      unsigned char* bytes, size_t length, kmldrawing::Image** image,
      bool will_be_used_for_geo = false);
   virtual void DrawImage(const kmldrawing::Image& image,
      float left, float top, float right, float bottom, float rotation);
   virtual void DrawImageGeo(const kmldrawing::Image& image,
      double west, double north, double east, double south);
   virtual void DrawEllipse(float x, float y, float x_axis, float y_axis,
      const kmldrawing::Pen& pen, const kmldrawing::Brush& brush);
   virtual void DrawPolyPolygon(
      const std::vector<std::pair<float, float>>& points,
      const std::vector<int>& point_counts,
      const kmldrawing::Pen& pen, const kmldrawing::Brush& brush);
   virtual void DrawLines(const std::vector<std::pair<float, float>>& points,
      const kmldrawing::Pen& pen);
   virtual void DrawString(const std::string& text, float x, float y,
      float size, const kmldrawing::Pen& pen);
   virtual void ShowInformation(
      const std::string& title, const std::string& text);
   virtual void SetTimer(int id, unsigned long ms);
   virtual void ClearAllTimers();
   virtual void Invalidate(void* ptr);
   virtual bool Fetch(const std::string& uri, std::string* data);
   virtual bool ShouldRender(const kmldom::TimePrimitivePtr& time_primitive);
   virtual void LogMessage(const std::string& message);

   int m_height, m_width;
   double m_ul_lat, m_ul_lon, m_lr_lat, m_lr_lon;

   std::stack<kmldrawing::Pen*> m_pens_to_delete_after_each_draw;
   std::stack<kmldrawing::Brush*> m_brushes_to_delete_after_each_draw;
   std::stack<kmldrawing::Image*> m_images_to_delete_on_overlay_close; // TODO: delete cached images

   JNIEnv* m_env;
   jobject m_obj;
};

#endif // #ifndef KML_TEST_SHIM_H_
