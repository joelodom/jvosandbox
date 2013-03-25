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

#include "kml_tests.h"
#include "kml/convenience/convenience.h"
#include <cmath>

int run_tests_main(int argc, char **argv) {
   ::testing::InitGoogleTest(&argc, argv);
   // gtest will delete new environment
   ::testing::AddGlobalTestEnvironment(new KMLTestEnvironment);
   return RUN_ALL_TESTS();
}

TEST(kml_drawing_library_tests, test_gtest)
{
   EXPECT_TRUE(true) << "law of contradiction failed";
}

TEST(kml_drawing_library_tests, test_fetch_kml_and_push_state)
{
   TestShim shim;
   kmldrawing::KMLDrawer drawer(&shim);
   ASSERT_TRUE(drawer.OpenKML(TEST_KML_NET_URL));
}

TEST(kml_drawing_library_tests, test_fetch_kml)
{
   // A KmlFile created by KmlCache retains a pointer back to this KmlCache,
   // which is used by the style resolution machinery to fetch any styleUrls
   // encountered.  A KmlFile created directly from KmlFile will not have a
   // KmlCache to use and any remote style or schema references will be quietly
   // ignored.

   kmldrawing::KMLNetFetcher net_fetcher;
   kmlengine::KmlCache kml_cache(&net_fetcher, 1234 /*max_size*/);

   // show that the KML cache can fetch KML from an absolute URL

   kmlengine::KmlFilePtr kml_file
      = kml_cache.FetchKmlAbsolute(TEST_KML_NET_URL);
   ASSERT_TRUE(kml_file != nullptr);

   std::string serialized1;
   kml_file->SerializeToString(&serialized1);

   // show that the KML cache can fetch KML from a relative URL

   kml_file
      = kml_cache.FetchKmlRelative(TEST_KML_NET_BASE, TEST_KML_NET_TARGET);
   ASSERT_TRUE(kml_file != nullptr);

   std::string serialized2;
   kml_file->SerializeToString(&serialized2);

   EXPECT_STREQ(serialized1.c_str(), serialized2.c_str());

   // show that the KML cache can fetch arbitrary data
   std::string arbitrary_data;
   ASSERT_TRUE(kml_cache.FetchDataRelative(
      TEST_ARBITRARY_DATA_BASE, TEST_ARBITRARY_DATA_TARGET, &arbitrary_data));
   EXPECT_STREQ(ARBITRARY_DATA.c_str(), arbitrary_data.c_str());
}

NON_UNIT_TEST(kml_drawing_library_tests, test_set_timer)
{
   TestShim shim;
   kmldrawing::KMLDrawer drawer(&shim);
   ASSERT_TRUE(drawer.OpenKML(TEST_REFRESH_ON_INTERVAL_KML));
   EXPECT_EQ((size_t)0, shim.m_set_timer_calls);
   drawer.Draw();
   EXPECT_EQ((size_t)1, shim.m_set_timer_calls);
}

class TestCacheBehaviorNetFetcher : public kmlbase::NetFetcher
{
public:
   bool m_should_fail;
   TestCacheBehaviorNetFetcher() : m_should_fail(true) {}

private:
   virtual bool FetchUrl(const std::string& url, std::string* data) const
   {
      if (m_should_fail)
         return false;
      *data = TEST_COORDINATES_KML_DATA;
      return true;
   }
};

TEST(kml_drawing_library_tests, test_cache_behavior)
{
   // The drawing library assumes that if a fetch fails, the KmlCache
   // will retry the URL if it is asked to fetch it again.  This test
   // validates that assumption.

   TestCacheBehaviorNetFetcher net_fetcher;
   kmlengine::KmlCache kml_cache(&net_fetcher, 10);

   // first we fail
   kmlengine::KmlFilePtr kml_file = kml_cache.FetchKmlAbsolute(
      TEST_KML_NET_URL);
   EXPECT_TRUE(kml_file == nullptr);

   // now we fetch
   net_fetcher.m_should_fail = false;
   kml_file = kml_cache.FetchKmlAbsolute(TEST_KML_NET_URL);
   ASSERT_TRUE(kml_file != nullptr);
   kmldom::ElementPtr root = kml_file->get_root();
   EXPECT_TRUE(root != nullptr);
}

TEST(kml_drawing_library_tests, test_parse_kml_datetime)
{
   std::string date_time, error;
   struct tm tm_val;
   int offset_hours, offset_minutes;
   bool negative_offset;

   date_time = "1997";
   EXPECT_TRUE(kmldrawing::ParseKMLDateTime(date_time, &tm_val, &offset_hours,
      &offset_minutes, &negative_offset, &error));
   EXPECT_EQ((size_t)0, error.length());
   EXPECT_EQ(97, tm_val.tm_year);
   EXPECT_EQ(0, tm_val.tm_mon);
   EXPECT_EQ(1, tm_val.tm_mday);
   EXPECT_EQ(0, tm_val.tm_hour);
   EXPECT_EQ(0, tm_val.tm_min);
   EXPECT_EQ(0, tm_val.tm_sec);
   EXPECT_EQ(0, offset_hours);
   EXPECT_EQ(0, offset_minutes);
   EXPECT_FALSE(negative_offset);

   date_time = "1997-07";
   EXPECT_TRUE(kmldrawing::ParseKMLDateTime(date_time, &tm_val, &offset_hours,
      &offset_minutes, &negative_offset, &error));
   EXPECT_EQ((size_t)0, error.length());
   EXPECT_EQ(97, tm_val.tm_year);
   EXPECT_EQ(6, tm_val.tm_mon);
   EXPECT_EQ(1, tm_val.tm_mday);
   EXPECT_EQ(0, tm_val.tm_hour);
   EXPECT_EQ(0, tm_val.tm_min);
   EXPECT_EQ(0, tm_val.tm_sec);
   EXPECT_EQ(0, offset_hours);
   EXPECT_EQ(0, offset_minutes);
   EXPECT_FALSE(negative_offset);

   date_time = "1997-07-16";
   EXPECT_TRUE(kmldrawing::ParseKMLDateTime(date_time, &tm_val, &offset_hours,
      &offset_minutes, &negative_offset, &error));
   EXPECT_EQ((size_t)0, error.length());
   EXPECT_EQ(97, tm_val.tm_year);
   EXPECT_EQ(6, tm_val.tm_mon);
   EXPECT_EQ(16, tm_val.tm_mday);
   EXPECT_EQ(0, tm_val.tm_hour);
   EXPECT_EQ(0, tm_val.tm_min);
   EXPECT_EQ(0, tm_val.tm_sec);
   EXPECT_EQ(0, offset_hours);
   EXPECT_EQ(0, offset_minutes);
   EXPECT_FALSE(negative_offset);

   date_time = "1997-07-16T07:30:15Z";
   EXPECT_TRUE(kmldrawing::ParseKMLDateTime(date_time, &tm_val, &offset_hours,
      &offset_minutes, &negative_offset, &error));
   EXPECT_EQ((size_t)0, error.length());
   EXPECT_EQ(97, tm_val.tm_year);
   EXPECT_EQ(6, tm_val.tm_mon);
   EXPECT_EQ(16, tm_val.tm_mday);
   EXPECT_EQ(7, tm_val.tm_hour);
   EXPECT_EQ(30, tm_val.tm_min);
   EXPECT_EQ(15, tm_val.tm_sec);
   EXPECT_EQ(0, offset_hours);
   EXPECT_EQ(0, offset_minutes);
   EXPECT_FALSE(negative_offset);

   date_time = "1997-07-16T10:30:15+03:00";
   EXPECT_TRUE(kmldrawing::ParseKMLDateTime(date_time, &tm_val, &offset_hours,
      &offset_minutes, &negative_offset, &error));
   EXPECT_EQ((size_t)0, error.length());
   EXPECT_EQ(97, tm_val.tm_year);
   EXPECT_EQ(6, tm_val.tm_mon);
   EXPECT_EQ(16, tm_val.tm_mday);
   EXPECT_EQ(10, tm_val.tm_hour);
   EXPECT_EQ(30, tm_val.tm_min);
   EXPECT_EQ(15, tm_val.tm_sec);
   EXPECT_EQ(3, offset_hours);
   EXPECT_EQ(0, offset_minutes);
   EXPECT_FALSE(negative_offset);
}

TEST(kml_drawing_library_tests, test_feature_and_ancestors_visible)
{
   kmldom::KmlFactory* factory = kmldom::KmlFactory::GetFactory();
   kmldom::FolderPtr folder = factory->CreateFolder();
   kmldom::PlacemarkPtr placemark = factory->CreatePlacemark();
   folder->add_feature(placemark);

   EXPECT_TRUE(kmldrawing::FeatureAndAncestorsVisible(folder));
   EXPECT_TRUE(kmldrawing::FeatureAndAncestorsVisible(placemark));

   folder->set_visibility(false);

   EXPECT_FALSE(kmldrawing::FeatureAndAncestorsVisible(folder));
   EXPECT_FALSE(kmldrawing::FeatureAndAncestorsVisible(placemark));
}

NON_UNIT_TEST(kml_drawing_library_tests, test_kmz_internal_space)
{
   // the KMZ file fetched below has an internal file with a space in it
   TestShim shim;
   kmldrawing::KMLDrawer drawer(&shim);
   ASSERT_TRUE(drawer.OpenKML(
      "http://osm-kml.appspot.com/static/internal_file_has_space.kmz"));
   EXPECT_EQ((size_t)0, shim.m_draw_image_calls);
   drawer.Draw();
   EXPECT_EQ((size_t)1, shim.m_draw_image_calls);
}

TEST(kml_drawing_library_tests, test_get_bbox_from_geometry)
{
   // create a KML file with a multi geometry
   std::string errors;
   kmlengine::KmlFilePtr kml_file = kmlengine::KmlFile::CreateFromParse(
      std::string(TEST_MULTI_GEOMETRY_KML_DATA), &errors);
   EXPECT_EQ((size_t)0, errors.length());
   ASSERT_TRUE(kml_file != nullptr);

   // drill down to the multi geometry
   kmldom::KmlPtr kml = kmldom::AsKml(kml_file->get_root());
   ASSERT_TRUE(kml != nullptr);
   kmldom::PlacemarkPtr placemark = kmldom::AsPlacemark(kml->get_feature());
   ASSERT_TRUE(placemark != nullptr);
   EXPECT_TRUE(placemark->has_geometry());

   // test GetBboxFromGeometry
   kmlengine::Bbox bbox;
   EXPECT_TRUE(
      kmldrawing::GetBboxFromGeometry(placemark->get_geometry(), &bbox));
   EXPECT_EQ(2.0, bbox.get_north());
   EXPECT_EQ(0.0, bbox.get_south());
   EXPECT_EQ(6.0, bbox.get_east());
   EXPECT_EQ(0.0, bbox.get_west());
}

TEST(kml_drawing_library_tests, test_quad_tile_random_points)
{
   // This is a rather inefficient probabilistic test, but it should
   // statistically excercise many of the decision branches in the
   // quad tile scheme.

   int NUM_HIT_TESTS = 1000;
   const int NUM_POINTS = 1000;
   const double LAT_MAX_MIN = 5.0;
   const double LON_MAX_MIN = 5.0;

   // generate random placemarks

   srand((unsigned int)time(nullptr));
   kmldrawing::QuadTile top_tile;

   kmldom::PlacemarkPtr placemarks[NUM_POINTS];

   for (size_t i = 0; i < NUM_POINTS; i++)
   {
      std::stringstream ss;
      ss << i;

      double lat = 2*LAT_MAX_MIN*rand()/RAND_MAX - LAT_MAX_MIN;
      double lon = 2*LON_MAX_MIN*rand()/RAND_MAX - LON_MAX_MIN;

      placemarks[i] = kmlconvenience::CreatePointPlacemark(ss.str(), lat, lon);

      top_tile.AddObject(placemarks[i]);
   }

   // hit test random points

   while (NUM_HIT_TESTS-- > 0)
   {
      double lat = 2*LAT_MAX_MIN*rand()/RAND_MAX - LAT_MAX_MIN;
      double lon = 2*LON_MAX_MIN*rand()/RAND_MAX - LON_MAX_MIN;

      kmldom::ObjectPtr obj;
      top_tile.FindNearest(lat, lon, &obj);

      kmldom::PlacemarkPtr placemark = kmldom::AsPlacemark(obj);
      ASSERT_TRUE(placemark != nullptr);

      // check it the slow way

      kmldom::PlacemarkPtr nearest;
      double nearest_distance = DBL_MAX;

      for (size_t i = 0; i < NUM_POINTS; i++)
      {
         const kmldom::PointPtr point
            = kmldom::AsPoint(placemarks[i]->get_geometry());
         const kmlbase::Vec3 vec3
            = point->get_coordinates()->get_coordinates_array_at(0);

         double dx2 = vec3.get_longitude() - lon;
         dx2 *= dx2;

         double dy2 = vec3.get_latitude() - lat;
         dy2 *= dy2;

         double d = ::sqrt(dx2 + dy2); // use real sqrt for testing

         if (d < nearest_distance)
         {
            nearest = placemarks[i];
            nearest_distance = d;
         }
      }

      EXPECT_STREQ(nearest->get_name().c_str(), placemark->get_name().c_str());
   }
}
