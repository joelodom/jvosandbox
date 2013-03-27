// Copyright (c) 1994-2013 Georgia Tech Research Corporation, Atlanta, GA
// This file is part of FalconView(R).

// FalconView(R) is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// FalconView(R) is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public License
// along with FalconView(R).  If not, see <http://www.gnu.org/licenses/>.

// FalconView(R) is a registered trademark of Georgia Tech Research Corporation.

#ifndef KML_TESTS_H_
#define KML_TESTS_H_

//#define ENABLE_NON_UNIT_TESTS

#ifdef ENABLE_NON_UNIT_TESTS
#pragma message("WARNING: Disable non-unit tests before commiting to SVN.")
#define NON_UNIT_TEST(test_case_name, test_name) TEST(test_case_name, test_name)
#else
#define NON_UNIT_TEST(test_case_name, test_name) \
   TEST(test_case_name, DISABLED_##test_name)
#endif

#include "gtest/gtest.h"
#include "kml/dom/xsd.h"
#include "kml/engine.h"
#include "KMLDrawingLibrary.h"

extern std::string TEST_KML;
extern std::string STYLES_KML;
extern std::string ARBITRARY_DATA;

extern const char* TEST_KML_NET_URL;
extern const char* TEST_KML_NET_BASE;
extern const char* TEST_KML_NET_TARGET;

extern const char* TEST_STYLES_KML_URL;

extern const char* TEST_ARBITRARY_DATA_URL;
extern const char* TEST_ARBITRARY_DATA_BASE;
extern const char* TEST_ARBITRARY_DATA_TARGET;

extern const char* TEST_VIEWFORMAT_URL;
extern std::string TEST_VIEWFORMAT_KML;

extern const char* TEST_HTTPQUERY_URL;
extern std::string TEST_HTTPQUERY1_KML_DATA;
extern std::string TEST_HTTPQUERY2_KML_DATA;
extern std::string TEST_HTTPQUERY3_KML_DATA;

extern std::string TEST_TREE_CONTROL_KML_DATA;

extern const char* SETTABLE_KML_URL;
extern std::string g_settable_kml;

//
// the following data may be saved with the save_all_test_kml "test"
//

static const char* TEST_COORDINATES_KML
   = "D:\\exclude\\temp\\coordinates.kml";
extern std::string TEST_COORDINATES_KML_DATA;

static const char* TEST_OUTER_BOUNDARY_KML
   = "D:\\exclude\\temp\\outer_boundary.kml";
extern std::string TEST_OUTER_BOUNDARY_KML_DATA;

static const char* TEST_LINE_STRING_KML
   = "D:\\exclude\\temp\\line_string.kml";
extern std::string TEST_LINE_STRING_KML_DATA;

static const char* TEST_MULTI_GEOMETRY_KML
   = "D:\\exclude\\temp\\multi_geometry.kml";
extern std::string TEST_MULTI_GEOMETRY_KML_DATA;

static const char* TEST_ICON_STYLE_KML
   = "D:\\exclude\\temp\\icon_style.kml";
extern std::string TEST_ICON_STYLE_KML_DATA;

static const char* TEST_ICON_COLOR_KML
   = "D:\\exclude\\temp\\icon_color.kml";
extern std::string TEST_ICON_COLOR_KML_DATA;

static const char* TEST_POLY_STYLE_KML
   = "D:\\exclude\\temp\\poly_style.kml";
extern std::string TEST_POLY_STYLE_KML_DATA;

static const char* TEST_LINE_STYLE_KML
   = "D:\\exclude\\temp\\line_style.kml";
extern std::string TEST_LINE_STYLE_KML_DATA;

static const char* TEST_GROUND_OVERLAY_KML
   = "D:\\exclude\\temp\\ground_overlay.kml";
extern std::string TEST_GROUND_OVERLAY_KML_DATA;

static const char* TEST_SUPER_OVERLAY_KML
   = "D:\\exclude\\temp\\super_overlay.kml";
extern std::string TEST_SUPER_OVERLAY_KML_DATA;

static const char* TEST_NETWORK_LINK_KML
   = "D:\\exclude\\temp\\network_link.kml";
extern std::string TEST_NETWORK_LINK_KML_DATA;

static const char* TEST_RELATIVE_LOAD_KML
   = "D:\\exclude\\temp\\relative_load.kml";
extern std::string TEST_RELATIVE_LOAD_KML_DATA;

static const char* TEST_NETWORK_SHARED_STYLES_KML
   = "D:\\exclude\\temp\\network_shared_styles.kml";
extern std::string TEST_NETWORK_SHARED_STYLES_KML_DATA;

static const char* TEST_KMZ_WITHOUT_EXTENSION_KML
   = "D:\\exclude\\temp\\kmz_without_extension.kml";
extern std::string TEST_KMZ_WITHOUT_EXTENSION_KML_DATA;

static const char* TEST_SCREEN_OVERLAY_OVERLAY_XY_KML
   = "D:\\exclude\\temp\\screen_overlay_overlay_xy.kml";
extern std::string TEST_SCREEN_OVERLAY_OVERLAY_XY_KML_DATA;

static const char* TEST_SCREEN_OVERLAY_SCREEN_XY_KML
   = "D:\\exclude\\temp\\screen_overlay_screen_xy.kml";
extern std::string TEST_SCREEN_OVERLAY_SCREEN_XY_KML_DATA;

static const char* TEST_SCREEN_OVERLAY_ROTATION_KML
   = "D:\\exclude\\temp\\screen_overlay_rotation.kml";
extern std::string TEST_SCREEN_OVERLAY_ROTATION_KML_DATA;

static const char* TEST_SCREEN_OVERLAY_SIZE_KML
   = "D:\\exclude\\temp\\screen_overlay_size.kml";
extern std::string TEST_SCREEN_OVERLAY_SIZE_KML_DATA;

static const char* TEST_NETWORK_LINK_WINFORMAT_KML
   = "D:\\exclude\\temp\\network_link_winformat.kml";
extern std::string TEST_NETWORK_LINK_WINFORMAT_KML_DATA;

static const char* TEST_NETWORK_LINK_LOCAL_RELATIVE_KML
   = "D:\\exclude\\temp\\network_link_local_relative.kml";
extern std::string TEST_NETWORK_LINK_LOCAL_RELATIVE_KML_DATA;

static const char* TEST_KML_REDIRECT_KML
   = "D:\\exclude\\temp\\kml_redirect.kml";
extern std::string TEST_KML_REDIRECT_KML_DATA;

static const char* TEST_RELATIVE_KMZ_NESTED_KML
   = "D:\\exclude\\temp\\relative_kmz_nested.kml";
extern std::string TEST_RELATIVE_KMZ_NESTED_KML_DATA;

static const char* TEST_STYLE_MAP_KML
   = "D:\\exclude\\temp\\style_map.kml";
extern std::string TEST_STYLE_MAP_KML_DATA;

static const char* TEST_BALLOON_STYLE_KML
   = "D:\\exclude\\temp\\balloon_style.kml";
extern std::string TEST_BALLOON_STYLE_KML_DATA;

static const char* TEST_POLY_FILLS_KML
   = "D:\\exclude\\temp\\poly_fills.kml";
extern std::string TEST_POLY_FILLS_KML_DATA;

static const char* TEST_LABEL_STYLE_KML
   = "D:\\exclude\\temp\\label_style.kml";
extern std::string TEST_LABEL_STYLE_KML_DATA;

static const char* TEST_REFRESH_ON_INTERVAL_KML
   = "D:\\exclude\\temp\\refresh_on_interval.kml";
extern std::string TEST_REFRESH_ON_INTERVAL_KML_DATA;

static const char* TEST_POLY_INNER_RINGS_KML
   = "D:\\exclude\\temp\\poly_inner_rings.kml";
extern std::string TEST_POLY_INNER_RINGS_KML_DATA;

static const char* TEST_POLY_INNER_RINGS_NOFILL_KML
   = "D:\\exclude\\temp\\poly_inner_rings_nofill.kml";
extern std::string TEST_POLY_INNER_RINGS_NOFILL_KML_DATA;

static const char* TEST_TIME_KML
   = "D:\\exclude\\temp\\time.kml";
extern std::string TEST_TIME_KML_DATA;

static const char* TEST_PHOTO_OVERLAY_KML
   = "D:\\exclude\\temp\\photo_overlay.kml";
extern std::string TEST_PHOTO_OVERLAY_KML_DATA;

static const char* TEST_STYLE_WORK_AROUND_KML
   = "D:\\exclude\\temp\\style_work_around.kml";
extern std::string TEST_STYLE_WORK_AROUND_KML_DATA;

static const char* TEST_HIT_TEST_INVISIBLE_FOLDER_KML
   = "D:\\exclude\\temp\\hit_test_invisible_folder.kml";
extern std::string TEST_HIT_TEST_INVISIBLE_FOLDER_KML_DATA;

static const char* ONLY_TIMESTAMPS_KML
   = "D:\\exclude\\temp\\only_timestamps.kml";
extern std::string ONLY_TIMESTAMPS_KML_DATA;

static const char* TEST_SCREEN_OVERLAY_WIN_PATH_KML
   = "D:\\exclude\\temp\\win_path.kml";
extern std::string TEST_SCREEN_OVERLAY_WIN_PATH_KML_DATA;

static const char* TEST_EXPLICIT_STYLE_URL_KML
   = "D:\\exclude\\temp\\explicit_style_url.kml";
extern std::string TEST_EXPLICIT_STYLE_URL_KML_DATA;

static const char* TEST_MODEL_KML = "D:\\exclude\\temp\\model.kml";
extern std::string TEST_MODEL_KML_DATA;

//
// end data that may be saved with the save_all_test_kml "test"
//

extern bool FetchAsTestDataImpl(const std::string& url, std::string* data);

class KMLTestEnvironment : public ::testing::Environment
{
public:
   virtual ~KMLTestEnvironment() {}

   virtual void SetUp()
   {
      kmldrawing::FetchAsTestData = FetchAsTestDataImpl;
      kmldrawing::InTestEnvironment = true;

      m_factory = kmldom::KmlFactory::GetFactory();
      m_xsd = kmldom::Xsd::GetSchema();

      // parse the test KML once here
      TEST_KML_ROOT = kmldom::Parse(TEST_KML, NULL);
      ASSERT_TRUE(TEST_KML_ROOT != nullptr);

      // build a map of test file names to test data

      kmldrawing::TestDataMap[TEST_COORDINATES_KML] = TEST_COORDINATES_KML_DATA;
      kmldrawing::TestDataMap[TEST_OUTER_BOUNDARY_KML]
         = TEST_OUTER_BOUNDARY_KML_DATA;
      kmldrawing::TestDataMap[TEST_LINE_STRING_KML] = TEST_LINE_STRING_KML_DATA;
      kmldrawing::TestDataMap[TEST_MULTI_GEOMETRY_KML]
         = TEST_MULTI_GEOMETRY_KML_DATA;
      kmldrawing::TestDataMap[TEST_ICON_STYLE_KML] = TEST_ICON_STYLE_KML_DATA;
      kmldrawing::TestDataMap[TEST_ICON_COLOR_KML] = TEST_ICON_COLOR_KML_DATA;
      kmldrawing::TestDataMap[TEST_POLY_STYLE_KML] = TEST_POLY_STYLE_KML_DATA;
      kmldrawing::TestDataMap[TEST_LINE_STYLE_KML] = TEST_LINE_STYLE_KML_DATA;
      kmldrawing::TestDataMap[TEST_GROUND_OVERLAY_KML]
         = TEST_GROUND_OVERLAY_KML_DATA;
      kmldrawing::TestDataMap[TEST_SUPER_OVERLAY_KML]
         = TEST_SUPER_OVERLAY_KML_DATA;
      kmldrawing::TestDataMap[TEST_NETWORK_LINK_KML]
         = TEST_NETWORK_LINK_KML_DATA;
      kmldrawing::TestDataMap[TEST_RELATIVE_LOAD_KML]
         = TEST_RELATIVE_LOAD_KML_DATA;
      kmldrawing::TestDataMap[TEST_NETWORK_SHARED_STYLES_KML]
         = TEST_NETWORK_SHARED_STYLES_KML_DATA;
      kmldrawing::TestDataMap[TEST_KMZ_WITHOUT_EXTENSION_KML]
         = TEST_KMZ_WITHOUT_EXTENSION_KML_DATA;
      kmldrawing::TestDataMap[TEST_SCREEN_OVERLAY_OVERLAY_XY_KML]
         = TEST_SCREEN_OVERLAY_OVERLAY_XY_KML_DATA;
      kmldrawing::TestDataMap[TEST_SCREEN_OVERLAY_SCREEN_XY_KML]
         = TEST_SCREEN_OVERLAY_SCREEN_XY_KML_DATA;
      kmldrawing::TestDataMap[TEST_SCREEN_OVERLAY_ROTATION_KML]
         = TEST_SCREEN_OVERLAY_ROTATION_KML_DATA;
      kmldrawing::TestDataMap[TEST_SCREEN_OVERLAY_SIZE_KML]
         = TEST_SCREEN_OVERLAY_SIZE_KML_DATA;
      kmldrawing::TestDataMap[TEST_NETWORK_LINK_WINFORMAT_KML]
         = TEST_NETWORK_LINK_WINFORMAT_KML_DATA;
      kmldrawing::TestDataMap[TEST_NETWORK_LINK_LOCAL_RELATIVE_KML]
         = TEST_NETWORK_LINK_LOCAL_RELATIVE_KML_DATA;
      kmldrawing::TestDataMap[TEST_KML_REDIRECT_KML]
         = TEST_KML_REDIRECT_KML_DATA;
      kmldrawing::TestDataMap[TEST_RELATIVE_KMZ_NESTED_KML]
         = TEST_RELATIVE_KMZ_NESTED_KML_DATA;
      kmldrawing::TestDataMap[TEST_STYLE_MAP_KML] = TEST_STYLE_MAP_KML_DATA;
      kmldrawing::TestDataMap[TEST_BALLOON_STYLE_KML]
         = TEST_BALLOON_STYLE_KML_DATA;
      kmldrawing::TestDataMap[TEST_POLY_FILLS_KML] = TEST_POLY_FILLS_KML_DATA;
      kmldrawing::TestDataMap[TEST_LABEL_STYLE_KML] = TEST_LABEL_STYLE_KML_DATA;
      kmldrawing::TestDataMap[TEST_REFRESH_ON_INTERVAL_KML]
         = TEST_REFRESH_ON_INTERVAL_KML_DATA;
      kmldrawing::TestDataMap[TEST_POLY_INNER_RINGS_KML]
         = TEST_POLY_INNER_RINGS_KML_DATA;
      kmldrawing::TestDataMap[TEST_POLY_INNER_RINGS_NOFILL_KML]
         = TEST_POLY_INNER_RINGS_NOFILL_KML_DATA;
      kmldrawing::TestDataMap[TEST_TIME_KML] = TEST_TIME_KML_DATA;
      kmldrawing::TestDataMap[TEST_PHOTO_OVERLAY_KML]
         = TEST_PHOTO_OVERLAY_KML_DATA;
      kmldrawing::TestDataMap[TEST_STYLE_WORK_AROUND_KML]
         = TEST_STYLE_WORK_AROUND_KML_DATA;
      kmldrawing::TestDataMap[TEST_HIT_TEST_INVISIBLE_FOLDER_KML]
         = TEST_HIT_TEST_INVISIBLE_FOLDER_KML_DATA;
      kmldrawing::TestDataMap[ONLY_TIMESTAMPS_KML] = ONLY_TIMESTAMPS_KML_DATA;
      kmldrawing::TestDataMap[TEST_SCREEN_OVERLAY_WIN_PATH_KML]
         = TEST_SCREEN_OVERLAY_WIN_PATH_KML_DATA;
      kmldrawing::TestDataMap[TEST_EXPLICIT_STYLE_URL_KML]
         = TEST_EXPLICIT_STYLE_URL_KML_DATA;
      kmldrawing::TestDataMap[TEST_MODEL_KML] = TEST_MODEL_KML_DATA;
   }

   virtual void TearDown()
   {
      // We use this testing environment to delete singleton objects in testing.
      // In a typical application, we would not do this.  We allow it here
      // since this will only happen after all of the tests are completed.  This
      // helps us to detect leaks.

      delete m_factory;
      delete m_xsd;
   }

   static bool verify_url_fetch_count()
   {
      // report a failure if we fetched the same URL more than once
      for (auto it = kmldrawing::URLFetchCount.begin();
         it != kmldrawing::URLFetchCount.end(); it++)
      {
         if (it->second != 1)
            return false;
      }
      return true;
   }

   static kmldom::ElementPtr TEST_KML_ROOT;

   static size_t s_viewformat_url_fetch_count;
   static size_t s_httpquery_url_fetch_count;

private:
   kmldom::KmlFactory* m_factory;
   kmldom::Xsd* m_xsd;
};

// A feature visitor can walk an entire kml hierarchy, or a subset of
// a KML hierarchy.
class TestFeatureVisitor : public kmlengine::FeatureVisitor
{
public:
   TestFeatureVisitor() : features_visited(0) {}

   virtual void VisitFeature(const kmldom::FeaturePtr& feature)
   {
      // ... per-feature code ...
      ++features_visited;
   }

   int features_visited;
};

class TestShimImage : public kmldrawing::Image
{
   virtual void GetSize(int* width, int* height) const
   {
      *width = 256;
      *height = 256;
   }
};

class TestShim : public kmldrawing::KMLShim
{
public:
   size_t m_set_timer_calls;
   size_t m_draw_image_calls;

   TestShim() : m_set_timer_calls(0), m_draw_image_calls(0) {}

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

private:
   TestShimImage m_image;
};

#endif // #ifndef KML_TESTS_H_
