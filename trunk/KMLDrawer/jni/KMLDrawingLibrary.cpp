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

#include "KMLDrawingLibrary.h"

#include "kml/base/uri_parser.h"
#include "kml/base/net_cache.h"
#include "kml/convenience/http_client.h"
#include "kml/convenience/convenience.h"
//#include <curl/curl.h>

#include <functional>
#include <cctype>
#include <locale>
#include <algorithm>
#include <fstream>
#include <float.h>
#include <set>
#include <cstring>

#ifdef _WIN32
#define strcasecmp _stricmp
#else
#include <strings.h>
#endif

bool kmldrawing::InTestEnvironment = false;
std::map<std::string, std::string> kmldrawing::TestDataMap;
std::map<std::string, unsigned int> kmldrawing::URLFetchCount;
bool (*kmldrawing::FetchAsTestData)(
   const std::string& url, std::string* data) = nullptr;

/*static*/ size_t kmldrawing::KMLDrawer::s_selected_placemark_draws = 0;
/*static*/ size_t kmldrawing::KMLDrawer::s_information_dialog_opens = 0;
/*static*/ size_t kmldrawing::KMLDrawer::s_explicit_uri_styles_resolved = 0;
/*static*/ size_t kmldrawing::KMLDrawer::s_colored_placemark_draws = 0;
/*static*/ std::string kmldrawing::KMLDrawer::s_last_balloon_text;
/*static*/ kmlbase::Color32 kmldrawing::KMLDrawer::s_white(0xffffffff);
/*static*/ kmlbase::Color32 kmldrawing::KMLDrawer::s_cyan(0xffffff00);
/*static*/ kmlbase::Color32 kmldrawing::KMLDrawer::s_clear(0x00ffffff);
/*static*/ char
   kmldrawing::KMLDrawer::s_currently_drawing_id[CURRENTLY_DRAWING_ID_LEN];

// thanks http://www.fatcow.com/free-icons
/*static*/ size_t kmldrawing::KMLDrawer::s_camera_bytes_len = 1548;
/*static*/ unsigned char* kmldrawing::KMLDrawer::s_camera_bytes =
   (unsigned char*)
   "\x89\x50\x4e\x47\xd\xa\x1a\xa\x0\x0\x0\xd\x49\x48\x44\x52\x0\x0\x0\x20\x0"
   "\x0\x0\x20\x8\x6\x0\x0\x0\x73\x7a\x7a\xf4\x0\x0\x0\x19\x74\x45\x58\x74"
   "\x53\x6f\x66\x74\x77\x61\x72\x65\x0\x41\x64\x6f\x62\x65\x20\x49\x6d\x61"
   "\x67\x65\x52\x65\x61\x64\x79\x71\xc9\x65\x3c\x0\x0\x5\xae\x49\x44\x41\x54"
   "\x78\xda\xec\x57\x6b\x68\x5c\x45\x14\xfe\xee\xdd\xbb\xaf\x6c\x76\xb3\x77"
   "\x5f\x49\xf3\x30\x6d\x23\xa5\xd5\x4\x2b\xfe\x6a\x9a\xb6\x4a\xb\x16\x8b\x56"
   "\xb4\x3f\x14\x41\x10\xa5\x2a\x88\x8a\xf8\x4b\x44\x4a\xf1\x5f\x45\x50\x14"
   "\x5f\xf4\xaf\x14\xac\x60\xd1\x96\x82\x2d\x4a\x4c\x1f\xa0\x4d\xd3\xc6\x36"
   "\x58\x6c\xd2\x34\xad\x6d\x92\x7d\xef\x26\xbb\xd9\xfb\x58\xcf\x99\xbb\x77"
   "\xbb\x49\xb6\xd6\x40\x21\x7f\x3a\xe1\xec\x4e\xce\x9c\x99\xf9\xe6\x3b\x67"
   "\xce\x9c\x95\xca\xe5\x32\x96\xb3\xc9\x58\xe6\x76\xf\x80\xc2\x1f\x92\x24"
   "\x2d\x79\xde\xa7\x9f\x7d\x71\xd0\xe1\x70\xec\x34\x4d\x8e\x21\x3b\x8e\x24"
   "\xc8\xb2\x4\xc3\x30\xe\xbd\xf9\xc6\xeb\xbb\x48\xa1\xdf\x69\x21\x89\x83\xb0"
   "\xe\x0\xe5\xa3\x8f\x3f\x39\xe0\xf1\x78\x9e\xb5\x15\xc5\x62\xf1\xfb\x77\xdf"
   "\x79\xeb\x39\xde\x6d\xcf\xde\xf\xf\x6c\x7d\xec\xd1\x5d\x7d\x7d\x1b\xe9\x3f"
   "\x73\xc1\x8a\x32\x6\x6\x4e\xe0\xf8\x2f\xbf\x1e\xdc\xf3\xc1\xfb\x6c\x6f\xfc"
   "\x1f\x0\xca\x97\x5f\xef\xff\x81\xbe\x77\x58\x27\x62\x56\xac\x13\x59\xcd\xd2"
   "\x59\x17\xa6\x8c\xf5\xf\xf5\x40\x2f\xcd\x61\x26\x97\x6\xcc\x5\x0\x64\x19"
   "\x3e\x7f\x10\x8a\xcb\x8d\xa1\x73\xc3\x35\x6b\xd8\xc3\x12\xad\x53\x3e\xfc"
   "\xda\xee\x97\x9f\x66\x86\x94\x8a\xde\x4b\x1b\xee\xd8\xfd\xca\x4b\x62\x17"
   "\xfe\x93\x2a\x13\xb9\x6f\xd3\xcc\xba\x44\x32\x81\x99\x7c\x1e\xc9\x44\x2"
   "\x89\xd5\x4f\xc2\x81\x79\x1e\x10\xc7\x8d\xde\xf8\x1d\xa1\x70\x18\xbb\x9e"
   "\x79\xa\x91\x70\xc4\xc2\x50\x1\xcf\xc\x7d\xf5\xcd\xfe\x1d\xbc\x27\x49\xce"
   "\x6\xe0\x91\x65\x5\xb9\xfc\xc\x2e\x8f\x8d\x21\x14\xc\x22\x97\xcd\x22\x99"
   "\x4a\x23\x9e\x48\x62\xfb\xe3\xdb\x70\x65\xfc\xaa\x70\x95\x10\x9a\x10\xe"
   "\xab\x38\x72\x1d\x2\x40\x2d\x4f\xc\x60\x7d\x54\xc5\x9c\x61\x22\x99\xce"
   "\xe2\xcf\xb\x17\x51\x24\xa6\x78\xcc\xd3\xd8\x84\xcd\x9b\x37\x41\xd7\x85"
   "\x57\x3c\xb5\x0\xa4\xc1\xc1\x3f\x70\xfe\xfc\x39\xb8\x5c\x4e\x90\xef\x85"
   "\x74\x77\x77\x43\x51\x14\xc1\x80\x69\x18\x55\x0\xdc\x2\x74\xb2\xf7\xda\xa"
   "\x90\x17\x84\xf\x93\x95\xc9\xab\xb8\x39\x15\x17\xc0\xa2\x21\x15\x87\xfd"
   "\x5b\xc4\xd8\x13\xca\x39\x68\x9a\x81\x4a\xf2\x93\xaa\xb7\x80\x9b\xd3\xe9"
   "\xa4\xcd\x5d\xd8\xb4\x65\x13\x86\x6\x87\x44\x7f\x74\x74\x14\x6b\xd7\xae"
   "\x25\x0\x6\x89\x49\xfe\x93\x5\x0\x99\xe4\xaf\xbf\x2f\xe3\xda\xc4\x35\xc4"
   "\xe3\xf1\x79\xc\x84\x23\x11\x74\x74\xb4\x23\xd8\x14\x14\xf3\xc2\xd1\x18"
   "\xde\xe\xe4\xc5\x58\x32\x13\xa3\x83\x68\xa8\xcd\xbe\x55\x0\xf6\xe9\x4e"
   "\x9f\x3c\x8d\x9e\x9e\x1e\x4c\x4c\x4c\x88\x8d\x58\x47\xd7\x8a\xc4\x8a\x3"
   "\xe\xa2\xb\x23\x23\x48\x25\xe2\xc2\x55\xab\xda\x57\x10\x4b\x56\x3a\xd1"
   "\x75\x13\xb3\xc5\x12\x2e\xc\xf\x23\xd6\xdc\x82\xae\xae\xfb\x31\x35\x9d"
   "\xc0\xe4\xe4\xb4\x88\x25\x3e\xb4\xaf\xd1\x57\x1f\x80\xdd\x36\xf4\x6e\xc0"
   "\xe0\x99\x41\xb4\xb6\xb5\x51\x94\xe7\x2c\x0\x9a\x5e\x59\x0\xb8\x74\xe9"
   "\x12\x32\xa9\x24\x42\xd1\x36\x1c\x1b\x73\x61\x28\xee\x46\xa1\x54\x89\x64"
   "\x17\xf9\x3f\x32\x87\x6d\xab\xbc\x88\x4f\x4f\xd2\x69\xcb\x58\xdd\xb5\x1a"
   "\x66\x25\x42\x79\x63\x83\x40\x96\x6b\xae\xee\x22\x0\xa7\x4e\x9e\xaa\x32"
   "\xe0\x22\xb7\x70\xd3\x88\x1\x8e\x81\x54\x2e\x8b\xc9\x9b\x37\xa0\xb6\xac"
   "\xc4\xe7\x67\x1b\xd1\xd2\x1a\xc3\xd6\xbe\x28\x5a\x62\x1\x61\x77\x73\x2a"
   "\x8b\xd1\xab\xd3\x34\x36\x85\x57\x1f\x76\x92\xed\x15\xa8\x14\xac\xfe\x46"
   "\xbf\x70\x21\x3\xd0\xd\xfd\xbf\x53\x31\x33\xc0\xa7\x54\x43\x21\x3b\x19\x8"
   "\x17\xf0\xf9\xaf\x8e\x4f\xa0\x6d\x45\xc\x3f\x8d\x35\x62\x5d\x77\x17\x7a"
   "\x1e\x58\x9\x6f\x83\x17\xe9\x5c\x49\x8\xf7\x59\xb7\xee\xc1\x2e\xfc\x3c"
   "\xd1\x88\xd6\x96\x98\x98\x63\xf2\xc9\x9\x0\xb\xdf\x80\x5a\x17\xc8\xf5\x18"
   "\x58\xb3\x66\xd\x52\xc9\x64\x35\x36\x74\x5d\x13\xb4\x4d\x4d\x4f\x21\x4a"
   "\x27\x82\xda\x81\xa6\xa6\x0\xb2\x33\x45\xe4\x67\xe7\x90\x27\x1f\x8\xa1\x3e"
   "\xeb\x9a\x82\x1\xcc\xf9\x3b\x10\x8b\xa8\x62\xe\xcf\xb5\x2\xd9\xa0\xb5\x74"
   "\xd8\xc9\xee\xce\xc\x54\x1a\x5f\x1d\xd3\x60\xa\xd\xb8\xdd\x6e\xb8\x1a\xfc"
   "\xc8\xd1\x66\xa5\x92\x2e\xc6\xf4\x8a\x70\x9f\x75\x3c\xc6\x36\x6c\xcb\xa9"
   "\x9a\xe7\x8a\xf9\x24\xba\x88\x27\x2c\x91\x1\x4d\x13\xf4\x71\x50\x39\x9d"
   "\xa\xa\x7a\x19\x4d\x34\xe6\xa7\x95\x7c\xb\x84\x75\x3c\xc6\x36\x6c\xcb\xb7"
   "\xa7\x4a\x3f\x51\xaf\x11\x3\x28\x2f\x95\x1\xa6\xcd\xa8\x44\x6f\x59\xc2\x1c"
   "\xbd\x3\x11\xc9\x81\x0\x45\x76\x3d\xe1\x31\xb6\x61\x5b\xda\x7e\x3e\x3\xb4"
   "\x56\xed\x2d\xb8\x23\x3\xdc\x4a\xa5\x12\x74\x53\x87\xd3\xe5\xc1\x24\xdd\xeb"
   "\xbe\xe6\x59\x2\x25\x43\x95\x9d\x24\xa\x82\xb2\x43\x8\xf7\x59\xc7\x63\x6c"
   "\xc3\xb6\x2e\xa7\x47\x44\xbe\x41\xfe\x67\xe1\xb5\xca\x4b\x61\x80\x5d\x50"
   "\x22\x17\x98\x74\x7f\xa3\xd1\x30\x86\x2f\x8e\x60\x67\x57\x1\x67\x52\x71"
   "\x8c\x67\x34\x4\x25\x27\x5a\x15\x8f\x10\xee\xb3\x8e\xc7\xd8\x86\x6d\x23"
   "\xd1\x8\xcd\xbd\xc5\x2\xaf\xb5\xa8\x20\xb9\x6d\x1e\xa0\x74\x5c\x65\x80\xae"
   "\xa2\xaa\x86\x29\xaa\xa7\x71\xf4\x58\x3f\xf6\x6d\xdf\x8a\x6f\x47\x12\xd8"
   "\x3b\x6c\x60\xb6\x92\x88\x1a\xc8\xfc\xf9\x1e\x7\xf6\xad\xd3\xf1\xe3\xd1"
   "\x7e\x4a\x40\xc4\x92\x1a\xa2\x3c\x72\x2b\xf9\x68\xda\x6d\x52\x31\x2b\x59"
   "\x98\x81\xb3\x83\x67\xc5\x63\x64\x1b\x32\x0\x83\x68\x54\x14\x7\xda\x5a"
   "\xdb\xf1\xcf\xf5\x6b\xf8\xee\xd0\x11\x6c\x7c\x64\x3d\x5e\x7c\xa1\x13\x5e"
   "\xb7\x95\xb0\xa\x73\x1a\xbd\xa6\xe3\x34\x36\x44\x73\x25\xca\xa6\xed\x62"
   "\x8e\x69\x6a\xd5\xb8\xe3\xb5\x6a\xaf\x61\x15\x80\x95\xef\xd\xc\xfc\x36"
   "\x20\xae\xf\x7\x1e\x3f\x44\xc\xc2\x7e\xc1\x38\x9b\x5\x2\x1\x4a\x2c\xad"
   "\x48\x50\x3d\x30\x70\xfa\xc\x8e\xff\x3a\x30\x8f\x41\xb7\xc7\x8b\x6\x9f"
   "\x8f\x9e\xeb\xb0\xb0\xe5\xf7\x41\x54\x15\x65\x2b\xaf\x69\x1a\x67\xc2\x3a"
   "\xa9\xb8\xb3\xb3\x13\xbd\xbd\xbd\x68\x6e\x8e\x8a\x97\x2c\x9b\xcd\x20\x95"
   "\xca\x20\x9d\xc9\x8\x0\x72\xcd\xbb\xab\x6\x55\xca\x7a\x3e\xe4\xa9\x66\x28"
   "\x14\x8b\xf3\x0\x78\x89\xb9\x46\xda\xd8\x43\xee\xb3\xaa\x2d\xeb\x1d\xe0"
   "\x6f\x8e\x27\x8b\x55\x69\x11\x80\x72\x3a\x9d\x3e\xd1\xdf\xdf\xbf\xd1\xce"
   "\xd9\xb5\xaf\x64\xbd\xc2\xb5\xb6\x36\x58\xd8\x6c\x77\xd6\xad\x1\x69\x4e"
   "\x3e\x9f\x3f\x61\xd7\x48\x76\x4d\xe8\xa7\xfe\x7d\x24\x91\x7a\x81\x79\x97"
   "\x1b\xfb\x20\xce\x4f\xb\x57\x44\xd5\xa2\xb4\x52\xa3\x79\x16\x55\x91\x77"
   "\xbf\xf1\xc9\xd9\x6f\x5\x1\x66\xb9\x7f\x1b\x4a\xf7\x7e\x9c\x2e\x37\x80"
   "\x7f\x5\x18\x0\xa0\x38\x3c\x5\x73\xe6\x7e\x7e\x0\x0\x0\x0\x49\x45\x4e"
   "\x44\xae\x42\x60\x82";

void kmldrawing::KML_DRAWING_ASSERT(bool b)
{
   //if (!b)
   //   throw std::exception();
}

void kmldrawing::KMLDrawer::Draw()
{
   // sanity checks
   KML_DRAWING_ASSERT(m_shim != nullptr);
   KML_DRAWING_ASSERT(m_kml_files.size() == 1);

   // clear timers
   m_shim->ClearAllTimers();

   // rebuild the quad tree during the draw
   m_quad_tree.Clear();

   // rebuild the cache (for auto refreshing KML)
   if (m_rebuild_cache_on_draw)
   {
      m_rebuild_cache_on_draw = false;
      DeselectSelectedFeature();
      delete m_kml_cache;
      m_kml_cache = new kmlengine::KmlCache(&m_net_fetcher, 1024);
      m_image_cache.clear();
      m_kml_file_cache.clear();
      m_kmz_file_cache.clear();
   }

   // save information about the map on which we are drawing
   m_shim->GetSurfaceSize(&m_surface_width, &m_surface_height);
   m_shim->GetMapBounds(&m_bottom_lat, &m_left_lon, &m_top_lat, &m_right_lon);
   m_shim->GetDegreesPerPixel(&m_dpp_y, &m_dpp_x);
   m_center_lat = (m_bottom_lat + m_top_lat) / 2.0;
   m_center_lon = (m_left_lon + m_right_lon) / 2.0;

   // create stock objects
   m_shim->CreatePen(s_white, 2.0f, &m_white_pen);
   m_shim->CreateBrush(s_white, &m_white_brush);
   m_shim->CreateBrush(s_clear, &m_clear_brush);
   m_stock_camera = nullptr;

   // clear the current pen and brush
   m_current_pen = nullptr;
   m_current_brush = nullptr;

   // draw
   m_draw_selected = false;
   m_shim->BeginDraw();
   RenderElement(m_kml_files.top()->get_root());
   m_shim->EndDraw();
   m_draw_selected = true;
   RenderElement(m_selected_feature, false); // draw selected placemark on top

   // sanity check (make sure everything was popped)
   KML_DRAWING_ASSERT(m_kml_files.size() == 1);
}

void kmldrawing::KMLDrawer::RenderElement(const kmldom::ElementPtr& element,
   bool add_to_quad_tree /*= true*/)
{
   if (element == nullptr)
      return;

   // some code for debugging only
#if 0
#pragma message("WARNING: Disable this code before commiting to SVN.")
   s_currently_drawing_id[0] = '\0';
   kmldom::ObjectPtr obj = kmldom::AsObject(element);
   if (obj != nullptr)
   {
      std::string id = obj->get_id();
      if (id.length() > 0 && id.length() < CURRENTLY_DRAWING_ID_LEN)
         strcpy_s(s_currently_drawing_id, CURRENTLY_DRAWING_ID_LEN, id.c_str());
   }
   // sample condition:
   //!strcmp(kmldrawing::KMLDrawer::s_currently_drawing_id, "pm150")
#endif

   m_shim->BeginDrawElement(element);

   // possibly add the element to the quad tree
   if (add_to_quad_tree)
      m_quad_tree.AddObject(kmldom::AsObject(element));

   // draw the element based on its type

   kmldom::KmlDomType type = element->Type();

   switch (type)
   {
   case kmldom::Type_Placemark:
   case kmldom::Type_GroundOverlay:
   case kmldom::Type_NetworkLink:
   case kmldom::Type_ScreenOverlay:
   case kmldom::Type_PhotoOverlay:
      RenderFeature(kmldom::AsFeature(element));
      break;
   case kmldom::Type_Document:
   case kmldom::Type_Folder:
      {
         kmldom::ContainerPtr container = kmldom::AsContainer(element);
         if (container->get_visibility())
         {
            size_t size = container->get_feature_array_size();
            for (size_t i = 0; i < size; ++i)
               RenderElement(container->get_feature_array_at(i));
         }
      }
      break;
   case kmldom::Type_kml:
      {
         kmldom::KmlPtr kml = kmldom::AsKml(element);
         if (kml->has_feature())
            RenderElement(kml->get_feature());
      }
      break;
   default:
      break;
   }

   m_shim->EndDrawElement(element);
}

void kmldrawing::KMLDrawer::RenderFeature(const kmldom::FeaturePtr& feature)
{
   // if feature is out of view, then (potentially) unload its
   // resources from the cache

   if (feature->get_visibility() == false)
      return;

   if (KMLFeatureInRegion(feature) == false)
      return;

   if (feature->has_timeprimitive()
      && !m_shim->ShouldRender(feature->get_timeprimitive()))
      return;

   // the selected feature will be drawn last so that it is on top
   if (!m_draw_selected && feature == m_selected_feature)
      return;

   kmldom::KmlDomType type = feature->Type();
   switch (type)
   {
   case kmldom::Type_Placemark:
      {
         RenderPlacemark(kmldom::AsPlacemark(feature));
         break;
      }
   case kmldom::Type_GroundOverlay: // <GroundOverlay>
      RenderGroundOverlay(kmldom::AsGroundOverlay(feature));
      break;
   case kmldom::Type_NetworkLink:
      RenderNetworkLink(kmldom::AsNetworkLink(feature));
      break;
   case kmldom::Type_ScreenOverlay: // <ScreenOverlay>
      RenderScreenOverlay(kmldom::AsScreenOverlay(feature));
      break;
   case kmldom::Type_PhotoOverlay: // <PhotoOverlay>
      RenderPhotoOverlay(kmldom::AsPhotoOverlay(feature));
      break;
   default:
      break;
   }
}

kmldom::ElementPtr kmldrawing::KMLDrawer::GetAncestorOfType(
   const kmldom::ElementPtr& element, kmldom::KmlDomType type)
{
   kmldom::ElementPtr ret = element->GetParent();
   while (ret != nullptr && !ret->IsA(type))
      ret = ret->GetParent();
   return ret;
}

bool kmldrawing::LongitudeInRange(
   double western_lon, double eastern_lon, double point_lon)
{
   if (western_lon <= eastern_lon)
   {
      if (point_lon < western_lon || eastern_lon < point_lon)
         return false;
   }
   else
   {
      if (point_lon < western_lon && point_lon > eastern_lon)
         return false;
   }

   return true;
}

bool kmldrawing::DoGeoRectsIntersect(
   double ll_A_lat, double ll_A_lon,
   double ur_A_lat, double ur_A_lon,
   double ll_B_lat, double ll_B_lon,
   double ur_B_lat, double ur_B_lon)
{
   if (ur_A_lat <= ll_B_lat)   /* region A completely below B */
      return false;

   if (ll_A_lat >= ur_B_lat)   /* region A completely above B */
      return false;

   /* if either region goes around the world, intersection exists */
   if (ll_A_lon == ur_A_lon || ll_B_lon == ur_B_lon)
      return true;

   /* if right edge of region B is in region A */
   if (LongitudeInRange(ll_A_lon, ur_A_lon, ur_B_lon))
      return true;

   /* if left edge of region B is in region A */
   if (LongitudeInRange(ll_A_lon, ur_A_lon, ll_B_lon))
      return true;

   /* if right edge of region A is in region B */
   if (LongitudeInRange(ll_B_lon, ur_B_lon, ur_A_lon))
      return true;

   /* if left edge of region A is in region B */
   if (LongitudeInRange(ll_B_lon, ur_B_lon, ll_A_lon))
      return true;

   return false;
}

bool kmldrawing::KMLDrawer::KMLFeatureInRegion(kmldom::FeaturePtr feature)
{
   // If the given feature does not have a region, then find the youngest
   // ancestor that does
   while (feature != nullptr && !feature->has_region())
      feature = kmldom::AsFeature(GetAncestorOfType(
      feature, kmldom::Type_Feature));

   if (feature == nullptr || !feature->has_region())
      return true; // no region, so active

   kmldom::RegionPtr region = feature->get_region();

   // return false if the <LatLonAltBox> is not in view

   if (!region->has_latlonaltbox()) // technically a required element
      return true; // assume active

   kmldom::LatLonAltBoxPtr bbox = region->get_latlonaltbox();

   if (!bbox->has_north()
      || !bbox->has_south()
      || !bbox->has_east()
      || !bbox->has_west())
      return true; // assume active

   double minX = bbox->get_west();
   double minY = bbox->get_south();
   double maxX = bbox->get_east();
   double maxY = bbox->get_north();

   if (!DoGeoRectsIntersect(
      minX, minY, maxX, maxY,
      m_left_lon, m_bottom_lat, m_right_lon, m_top_lat))
      return false; // not active

   // return false if the <Lod> requirements aren't met

   if (!region->has_lod())
      return true; // active

   kmldom::LodPtr lod = region->get_lod();

   // compute the square root of the area, in square pixels, of the
   // region projected onto the surface
   double regionAreaPixels
      = sqrt(((maxX - minX) / m_dpp_x)
      * ((maxY - minY) / m_dpp_y));

   // test <minLodPixels>
   if (lod->has_minlodpixels())
   {
      double minLodPixels = lod->get_minlodpixels();
      if (regionAreaPixels < minLodPixels)
         return false; // not active
   }

   // test <maxLodPixels>
   if (lod->has_maxlodpixels())
   {
      double maxLodPixels = lod->get_maxlodpixels();
      if (maxLodPixels > 0) // -1 means active to infinite size on screen
      {
         if (regionAreaPixels > maxLodPixels)
            return false; // not active
      }
   }

   return true; // region is active
}

void kmldrawing::KMLDrawer::RenderPlacemark(
   const kmldom::PlacemarkPtr& placemark)
{
   if (!placemark->has_geometry())
      return;

   const kmldom::GeometryPtr& geometry = placemark->get_geometry();
   RenderGeometry(geometry, placemark);
}

void kmldrawing::KMLDrawer::RenderGeometry(const kmldom::GeometryPtr& geometry,
   const kmldom::PlacemarkPtr& associated_placemark)
{
   kmldom::KmlDomType type = geometry->Type();
   switch (type)
   {
   case kmldom::Type_MultiGeometry:
      {
         const kmldom::MultiGeometryPtr multi_geometry
            = kmldom::AsMultiGeometry(geometry);
         size_t size = multi_geometry->get_geometry_array_size();
         for (size_t i = 0; i < size; i++)
            RenderGeometry(multi_geometry->get_geometry_array_at(i),
            associated_placemark);
      }
      break;

   case kmldom::Type_Point:
      RenderPoint(kmldom::AsPoint(geometry), associated_placemark);
      break;
   case kmldom::Type_Polygon:
      RenderPolygon(kmldom::AsPolygon(geometry), associated_placemark);
      break;
   case kmldom::Type_LineString:
      RenderLineString(kmldom::AsLineString(geometry), associated_placemark);
      break;
   case kmldom::Type_Model:
      {
         // draw a point where the model would be

         kmldom::ModelPtr model = kmldom::AsModel(geometry);
         if (!model->has_location())
            break;
         const kmldom::LocationPtr location = model->get_location();
         if (!(location->has_latitude() && location->has_longitude()))
            break;

         double lat = location->get_latitude();
         double lon = location->get_longitude();

         kmldom::PlacemarkPtr placemark = kmlconvenience::CreatePointPlacemark(
            associated_placemark->get_name(), lat, lon);
         RenderGeometry(kmldom::AsPoint(placemark->get_geometry()), placemark);
      }

      break;
   default:
      break;
   }
}

void kmldrawing::KMLDrawer::RenderPoint(const kmldom::PointPtr& point,
   const kmldom::PlacemarkPtr& associated_placemark)
{
   const kmldom::CoordinatesPtr coordinates = point->get_coordinates();
   const kmlbase::Vec3 vec = coordinates->get_coordinates_array_at(0);

   const double latitude = vec.get_latitude();
   const double longitude = vec.get_longitude();

   Pen* pen = m_white_pen;
   Brush* brush = m_white_brush;

   if (associated_placemark == m_selected_feature)
   {
      // use cyan pen and brush if placemark selected
      SetPen(s_cyan, 3.0f, &pen);
      SetBrush(s_cyan, &brush);
      ++s_selected_placemark_draws;
   }

   int surface_x, surface_y;
   m_shim->GeoToSurface(latitude, longitude, &surface_x, &surface_y);
   if (surface_x >= 0 && surface_x < m_surface_width &&
      surface_y >= 0 && surface_y < m_surface_height)
   {
      kmldom::StylePtr style = ResolveStyle(associated_placemark);

      float icon_size = 4.0f;

      if (style != nullptr && style->has_iconstyle())
      {
         kmldom::IconStylePtr icon_style = style->get_iconstyle();

         if (icon_style->has_icon())
         {
            kmldom::IconStyleIconPtr icon = icon_style->get_icon();
            if (icon->has_href())
            {
               // get the rotation
               float rotation = icon_style->has_heading()
                  ? (float)icon_style->get_heading() : 0.0f;

               // fetch the image
               std::string href = icon->get_href();
               Image* img = nullptr;
               FetchImage(href, &img);
               if (img == nullptr)
                  return;

               // determine the image scale

               const double NORMAL_MAX_SIZE = 50.0; // based on GE
               double scale = 1.0;

               int width, height;
               img->GetSize(&width, &height);

               if (width > height && width > NORMAL_MAX_SIZE)
                  scale = NORMAL_MAX_SIZE/width;
               else if (height > NORMAL_MAX_SIZE)
                  scale = NORMAL_MAX_SIZE/height;

               if (icon_style->has_scale())
                  scale *= icon_style->get_scale();

               // determine the image position

               float half_scale = (float)(scale / 2);

               float left = surface_x - half_scale*width;
               float top = surface_y - half_scale*height;
               float right = surface_x + half_scale*width;
               float bottom = surface_y + half_scale*height;

               // draw
               m_shim->DrawImage(*img, left, top, right, bottom, rotation);

               return;
            }
         }

         if (icon_style->has_color())
         {
            kmlbase::Color32 color = icon_style->get_color();
            SetPen(color, 1.0f, &pen);
            SetBrush(color, &brush);
            ++s_colored_placemark_draws;
         }

         if (icon_style->has_scale())
            icon_size *= (float)icon_style->get_scale();
      }

      m_shim->DrawEllipse((float)surface_x, (float)surface_y,
         icon_size, icon_size, *pen, *brush);

      if (style != nullptr && style->has_labelstyle()
         && associated_placemark->has_name())
      {
         // Draw any label.  Note that by the KML 2.2 standard, only
         // points are labeled.

         kmldom::LabelStylePtr label_style = style->get_labelstyle();
         pen = m_white_pen;
         std::string name = associated_placemark->get_name();
         float size = 18.0f;

         if (label_style->has_color())
         {
            kmlbase::Color32 color = label_style->get_color();
            SetPen(color, 2.0f, &pen);
         }

         if (label_style->has_scale())
            size *= (float)label_style->get_scale();

         if (size > 0.0f)
            m_shim->DrawString(name, (float)surface_x, (float)surface_y,
            size, *pen);
      }
   }
}

void kmldrawing::KMLDrawer::SetPenFromKMLLineStyle(
   kmldom::StylePtr style, Pen** current_pen)
{
   // If style is valid and has a line style, this will return a pen according
   // to the style.  Otherwise, this just trivially returns the current pen
   // unmodified.

   if (style != nullptr && style->has_linestyle())
   {
      kmlbase::Color32 color = s_white; // default to white
      float width = 2.0f; // default width

      kmldom::LineStylePtr line_style = style->get_linestyle();

      if (line_style->has_color())
         color = line_style->get_color();

      if (line_style->has_width())
         width = (float)line_style->get_width();

      SetPen(color, width, current_pen);
      return;
   }
}

bool kmldrawing::StyleAppearsResolved(const kmldom::StylePtr& style)
{
   return style->has_iconstyle()
      || style->has_linestyle()
      || style->has_labelstyle()
      || style->has_polystyle()
      || style->has_balloonstyle()
      || style->has_labelstyle();
}

kmldom::StylePtr kmldrawing::KMLDrawer::ResolveStyle(
   const kmldom::PlacemarkPtr& placemark)
{
   // Per Google KML reference, inline styles take precedence over shared
   // styles.  CreateResolvedStyle uses the KML file, the KML cache, the inline
   // styles, etc. to figure out the proper styling for the placemark.

   kmlengine::KmlFilePtr kml_file;
   DeconstructedUri deconstructed;

   if (placemark->has_styleurl()
      && DeconstructUri(placemark->get_styleurl(), &deconstructed)
      && deconstructed.scheme.length() // if it looks like explicit URI
      && FetchKML(placemark->get_styleurl(), false, &kml_file))
   {
      // pull the KML file specified by the explicit URI
      ++s_explicit_uri_styles_resolved;
   }
   else
   {
      // typical case
      kml_file = m_kml_files.top();
   }

   kmldom::StylePtr style = kmlengine::CreateResolvedStyle(placemark,
      kml_file, placemark == m_selected_feature
      ? kmldom::STYLESTATE_HIGHLIGHT
      : kmldom::STYLESTATE_NORMAL);

   if (placemark->has_styleurl() && !StyleAppearsResolved(style))
   {
      // KML requires that shared styles be prefixed with a #, but, in typical
      // Google Earth fashion, GE will resolve a shared style without a hash,
      // so people tend to do it that way.  This is a work around that will
      // only happen where there is a style URL that appears unresolvable.

      std::string original_style_url = placemark->get_styleurl();
      std::string new_style_url("#");
      new_style_url += original_style_url;

      placemark->set_styleurl(new_style_url);
      style = kmlengine::CreateResolvedStyle(placemark,
         m_kml_files.top(), placemark == m_selected_feature
         ? kmldom::STYLESTATE_HIGHLIGHT
         : kmldom::STYLESTATE_NORMAL);

      if (!StyleAppearsResolved(style))
      {
         // it didn't work
         placemark->set_styleurl(original_style_url);
      }
   }

   // KML 2.2 draws labels by default

   if (!style->has_labelstyle())
   {
      kmldom::KmlFactory* factory = kmldom::KmlFactory::GetFactory();
      kmldom::LabelStylePtr label_style = factory->CreateLabelStyle();
      label_style->set_color(kmlbase::Color32(0xffffffff));
      label_style->set_colormode(kmldom::COLORMODE_NORMAL);
      label_style->set_scale(1.0);
      style->set_labelstyle(label_style);
   }

   return style;
}

void kmldrawing::KMLDrawer::AddPointToVector(const kmlbase::Vec3& vec,
   std::vector<std::pair<float, float>>* points)
{
   double latitude = vec.get_latitude();
   double longitude = vec.get_longitude();

   int surface_x, surface_y;
   m_shim->GeoToSurface(latitude, longitude, &surface_x, &surface_y);

   std::pair<float, float> point;
   point.first = static_cast<float>(surface_x);
   point.second = static_cast<float>(surface_y);

   points->push_back(point);
}

void kmldrawing::KMLDrawer::RenderPolygon(const kmldom::PolygonPtr& polygon,
   const kmldom::PlacemarkPtr& associated_placemark)
{
   // drill down to the polygon coordinates
   if (!polygon->has_outerboundaryis()) // required element
      return;
   kmldom::OuterBoundaryIsPtr outer_boundary = polygon->get_outerboundaryis();
   if (!outer_boundary->has_linearring())
      return;
   kmldom::LinearRingPtr linear_ring = outer_boundary->get_linearring();
   if (!linear_ring->has_coordinates())
      return; // required element
   kmldom::CoordinatesPtr coordinates = linear_ring->get_coordinates();

   // make sure the polygon is on the screen

   size_t array_size = coordinates->get_coordinates_array_size() - 1;
   if (array_size < 2)
   {
      // KML polygons are explicitly closed, meaning that the last vertex
      // is the same as the first vertex.  For this purpose, we subtract
      // one above.  DrawPolyPolygon automatically closes the polygon.
      return;
   }

   kmlengine::Bbox bbox;

   for (size_t i = 0; i < array_size; i++)
   {
      kmlbase::Vec3 vec = coordinates->get_coordinates_array_at(i);
      double latitude = vec.get_latitude();
      double longitude = vec.get_longitude();
      bbox.ExpandLatLon(latitude, longitude);
   }

   double minX = bbox.get_west();
   double minY = bbox.get_south();
   double maxX = bbox.get_east();
   double maxY = bbox.get_north();

   if (!DoGeoRectsIntersect(
      minX, minY, maxX, maxY,
      m_left_lon, m_bottom_lat, m_right_lon, m_top_lat))
      return;

   // create the array with the polygon points

   std::vector<int> point_counts;
   std::vector<std::pair<float, float>> points;

   point_counts.push_back(array_size);

   for (size_t i = 0; i < array_size; i++)
   {
      kmlbase::Vec3 vec = coordinates->get_coordinates_array_at(i);
      AddPointToVector(vec, &points);
   }

   // add inner boundaries

   size_t inner_boundaries = polygon->get_innerboundaryis_array_size();

   for (size_t i = 0; i < inner_boundaries; i++)
   {
      kmldom::InnerBoundaryIsPtr inner_boundary
         = polygon->get_innerboundaryis_array_at(i);
      if (!inner_boundary->has_linearring())
         continue;
      kmldom::LinearRingPtr inner_linear_ring
         = inner_boundary->get_linearring();
      if (!inner_linear_ring->has_coordinates())
         continue;
      kmldom::CoordinatesPtr inner_coordinates
         = inner_linear_ring->get_coordinates();

      size_t inner_array_size
         = inner_coordinates->get_coordinates_array_size() - 1;
      if (inner_array_size < 2)
      {
         // KML polygons are explicitly closed, meaning that the last vertex
         // is the same as the first vertex.  For this purpose, we subtract
         // one above.  DrawPolyPolygon automatically closes the polygon.
         continue;
      }

      point_counts.push_back(inner_array_size);

      for (size_t j = 0; j < inner_array_size; j++)
      {
         kmlbase::Vec3 vec = inner_coordinates->get_coordinates_array_at(j);
         AddPointToVector(vec, &points);
      }
   }

   // process any associated style

   kmldom::StylePtr style = ResolveStyle(associated_placemark);

   Pen* pen = m_white_pen;
   Brush* brush = m_white_brush;

   if (associated_placemark == m_selected_feature)
   {
      // use cyan pen and brush if placemark selected
      SetPen(s_cyan, 3.0f, &pen);
      SetBrush(s_cyan, &brush);
      ++s_selected_placemark_draws;
   }

   SetPenFromKMLLineStyle(style, &pen);

   if (style != nullptr && style->has_polystyle())
   {
      kmldom::PolyStylePtr poly_style = style->get_polystyle();

      if (!poly_style->get_outline())
         SetPen(s_white, 0.0f, &pen); // no outline

      if (!poly_style->get_fill())
      {
         brush = m_clear_brush;
      }
      else if (poly_style->has_color())
      {
         kmlbase::Color32 color = poly_style->get_color();
         SetBrush(color, &brush);
      }
   }

   // draw
   m_shim->DrawPolyPolygon(points, point_counts, *pen, *brush);
}

void kmldrawing::KMLDrawer::RenderLineString(
   const kmldom::LineStringPtr& line_string,
   const kmldom::PlacemarkPtr& associated_placemark)
{
   // drill down to the line string coordinates
   if (!line_string->has_coordinates()) // required element
      return;
   kmldom::CoordinatesPtr coordinates = line_string->get_coordinates();

   // make sure the line is on the screen

   size_t array_size = coordinates->get_coordinates_array_size();
   if (array_size < 2)
      return;

   kmlengine::Bbox bbox;

   for (size_t i = 0; i < array_size; i++)
   {
      kmlbase::Vec3 vec = coordinates->get_coordinates_array_at(i);
      double latitude = vec.get_latitude();
      double longitude = vec.get_longitude();
      bbox.ExpandLatLon(latitude, longitude);
   }

   double minX = bbox.get_west();
   double minY = bbox.get_south();
   double maxX = bbox.get_east();
   double maxY = bbox.get_north();

   if (!DoGeoRectsIntersect(
      minX, minY, maxX, maxY,
      m_left_lon, m_bottom_lat, m_right_lon, m_top_lat))
      return;

   // create the array with the line points

   std::vector<std::pair<float, float>> points;

   for (size_t i = 0; i < array_size; i++)
   {
      kmlbase::Vec3 vec = coordinates->get_coordinates_array_at(i);
      AddPointToVector(vec, &points);
   }

   // process any associated style

   kmldom::StylePtr style = ResolveStyle(associated_placemark);

   Pen* pen = m_white_pen;

   if (associated_placemark == m_selected_feature)
   {
      // use cyan pen if placemark selected
      SetPen(s_cyan, 3.0f, &pen);
      ++s_selected_placemark_draws;
   }

   SetPenFromKMLLineStyle(style, &pen);

   // draw
   m_shim->DrawLines(points, *pen);
}

const kmldom::AbstractLatLonBoxPtr kmldrawing::KMLDrawer::GetLatLonBox(
   const kmldom::GroundOverlayPtr& groundoverlay)
{
   if (!groundoverlay->has_latlonbox())
   {
      // fix for specifying <LatLonAltBox>
      // instead of <LatLonBox> for ground overlay
      size_t size = groundoverlay->get_misplaced_elements_array_size();
      for (size_t i = 0; i < size; i++)
      {
         const kmldom::AbstractLatLonBoxPtr& latlonbox =
            kmldom::AsAbstractLatLonBox(
            groundoverlay->get_misplaced_elements_array_at(i));

         if (latlonbox != nullptr)
            return latlonbox;
      }

      return nullptr;
   }

   return groundoverlay->get_latlonbox();
}

void kmldrawing::KMLDrawer::RenderGroundOverlay(
   const kmldom::GroundOverlayPtr& ground_overlay)
{
   // fetch the overlay bounds

   if (!ground_overlay->has_latlonbox())
      return;

   kmldom::LatLonBoxPtr lat_lon_box = ground_overlay->get_latlonbox();

   if (!lat_lon_box->has_north()
      || !lat_lon_box->has_south()
      || !lat_lon_box->has_east()
      || !lat_lon_box->has_west())
      return;

   double north = lat_lon_box->get_north();
   double south = lat_lon_box->get_south();
   double east = lat_lon_box->get_east();
   double west = lat_lon_box->get_west();

   // make sure the overlay is on the screen
   if (!DoGeoRectsIntersect(
      west, south, east, north,
      m_left_lon, m_bottom_lat, m_right_lon, m_top_lat))
      return;

   // fetch the overlay image
   if (!ground_overlay->has_icon() || !ground_overlay->get_icon()->has_href())
      return;
   std::string href = ground_overlay->get_icon()->get_href();
   Image* img = nullptr;
   FetchImage(href, &img, true);
   if (img == nullptr)
      return;

   // draw
   m_shim->DrawImageGeo(*img, west, north, east, south);
}

void kmldrawing::KMLDrawer::RenderScreenOverlay(
   const kmldom::ScreenOverlayPtr& screen_overlay)
{
   // fetch the overlay image
   if (!screen_overlay->has_icon() || !screen_overlay->get_icon()->has_href())
      return;
   std::string href = screen_overlay->get_icon()->get_href();
   Image* img = nullptr;
   FetchImage(href, &img);
   if (img == nullptr)
      return;

   // determine the screen anchor (defaults to center in GE)

   float screen_x = (float)m_surface_width / 2.0f;
   float screen_y = (float)m_surface_height / 2.0f;

   if (screen_overlay->has_screenxy())
   {
      kmldom::ScreenXYPtr screenxy = screen_overlay->get_screenxy();

      if (screenxy->has_x()
         && screenxy->has_xunits()
         && screenxy->get_xunits() == kmldom::UNITS_FRACTION)
      {
         screen_x = (float)(m_surface_width * screenxy->get_x());
      }
      else if (screenxy->has_x()
         && screenxy->has_xunits()
         && screenxy->get_xunits() == kmldom::UNITS_INSETPIXELS)
      {
         // inset pixels are from upper right
         screen_x = (float)(m_surface_width - screenxy->get_x());
      }
      else if (screenxy->has_x())
      {
         // pixels
         screen_x = (float)screenxy->get_x();
      }

      if (screenxy->has_y()
         && screenxy->has_yunits()
         && screenxy->get_yunits() == kmldom::UNITS_FRACTION)
      {
         screen_y = (float)(m_surface_height * screenxy->get_y());
      }
      else if (screenxy->has_y()
         && screenxy->has_yunits()
         && screenxy->get_yunits() == kmldom::UNITS_INSETPIXELS)
      {
         // inset pixels are from upper right
         screen_y = (float)(m_surface_height - screenxy->get_y());
      }
      else if (screenxy->has_y())
      {
         // pixels
         screen_y = (float)screenxy->get_y();
      }
   }

   // determine the image size

   int i_width, i_height;
   img->GetSize(&i_width, &i_height);

   float width = (float)i_width;
   float height = (float)i_height;

   if (screen_overlay->has_size())
   {
      kmldom::SizePtr size = screen_overlay->get_size();

      // first set sizes skipping over zeros (zero maintains aspect ratio)

      if (size->has_x() && size->get_x() != 0.0)
      {
         if (size->has_xunits() && size->get_xunits() == kmldom::UNITS_FRACTION)
         {
            width = (float)(size->get_x() * m_surface_width);
         }
         else if (size->get_x() > 0.0) // -1 means maintain native size
         {
            width = (float)size->get_x();
         }
      }

      if (size->has_y() && size->get_y() != 0.0)
      {
         if (size->has_yunits() && size->get_yunits() == kmldom::UNITS_FRACTION)
         {
            height
               = (float)(size->get_y() * m_surface_height);
         }
         else if (size->get_y() > 0.0) // -1 means maintain native size
         {
            height = (float)size->get_y();
         }
      }

      // next set sizes taking into account zeros

      if (size->has_x() && size->get_x() == 0.0 && size->get_y() != 0.0)
         width *= height/i_height;

      if (size->has_y() && size->get_y() == 0.0 && size->get_x() != 0.0)
         height *= width/i_width;
   }

   // determine the image anchor (defaults to center in GE)

   float overlay_x = width / 2.0f;
   float overlay_y = height / 2.0f;

   if (screen_overlay->has_overlayxy())
   {
      kmldom::OverlayXYPtr overlayxy = screen_overlay->get_overlayxy();

      if (overlayxy->has_x()
         && overlayxy->has_xunits()
         && overlayxy->get_xunits() == kmldom::UNITS_FRACTION)
      {
         overlay_x = (float)(width * overlayxy->get_x());
      }
      else if (overlayxy->has_x()
         && overlayxy->has_xunits()
         && overlayxy->get_xunits() == kmldom::UNITS_INSETPIXELS)
      {
         // inset pixels are from upper right
         overlay_x = (float)(width - overlayxy->get_x());
      }
      else if (overlayxy->has_x())
      {
         // pixels
         overlay_x = (float)overlayxy->get_x();
      }

      if (overlayxy->has_y()
         && overlayxy->has_yunits()
         && overlayxy->get_yunits() == kmldom::UNITS_FRACTION)
      {
         overlay_y = (float)(height * overlayxy->get_y());
      }
      else if (overlayxy->has_y()
         && overlayxy->has_yunits()
         && overlayxy->get_yunits() == kmldom::UNITS_INSETPIXELS)
      {
         // inset pixels are from upper right
         overlay_y = (float)(height - overlayxy->get_y());
      }
      else if (overlayxy->has_y())
      {
         // pixels
         overlay_y = (float)overlayxy->get_y();
      }
   }

   // determine the drawing box
   // origin of FalconView screen is upper left

   float left = screen_x - overlay_x;
   float right = left + width;
   float bottom = (float)m_surface_height - (screen_y - overlay_y);
   float top = bottom - height;

   // determine the image rotation

   float rotation = screen_overlay->has_rotation()
      ? (float)screen_overlay->get_rotation() : 0.0f;

   if (screen_overlay->has_rotationxy())
   {
      // Google Earth uses this to rotate around a particular point
      // on the image.  I should be able to use linear algebra to determine
      // how to adjust the drawing box to account for this.
   }

   // draw (note change of rotation direction)
   m_shim->DrawImage(*img, left, top, right, bottom, -rotation);
}

void kmldrawing::KMLDrawer::RenderPhotoOverlay(
   const kmldom::PhotoOverlayPtr& photo_overlay)
{
   // draw photo overlays like placemarks symbolized by cameras

   if (!photo_overlay->has_point())
      return;

   const kmldom::PointPtr point = photo_overlay->get_point();
   const kmldom::CoordinatesPtr coordinates = point->get_coordinates();
   const kmlbase::Vec3 vec = coordinates->get_coordinates_array_at(0);

   const double latitude = vec.get_latitude();
   const double longitude = vec.get_longitude();

   int surface_x, surface_y;
   m_shim->GeoToSurface(latitude, longitude, &surface_x, &surface_y);
   if (surface_x >= 0 && surface_x < m_surface_width &&
      surface_y >= 0 && surface_y < m_surface_height)
   {
      Image* image = nullptr;
      int width, height;

      if (photo_overlay == m_selected_feature
         && photo_overlay->has_icon() && photo_overlay->get_icon()->has_href())
      {
         // If the photo overlay is the selected feature, we draw a thumbnail
         // of it on the FalconView map.  This mitigates the problem where
         // a photo is embedded in a KMZ and can't draw in the information
         // dialog.

         std::string href = photo_overlay->get_icon()->get_href();
         FetchImage(href, &image); // nullptr if failure fetching

         if (image != nullptr)
         {
            image->GetSize(&width, &height);

            const int MAX_SIZE = 100;
            if (width > MAX_SIZE || height > MAX_SIZE)
            {
               double max = width > height ? width : height; 
               double scale_factor = ((double)MAX_SIZE)/max;
               width = (int)(width*scale_factor);
               height = (int)(height*scale_factor);
            }
         }
      }

      if (image == nullptr)
      {
         if (m_stock_camera == nullptr)
         {
            m_shim->CreateImageFromRawBytes(
               s_camera_bytes, s_camera_bytes_len, &m_stock_camera);
            if (m_stock_camera == nullptr)
               return; // what happened?
         }

         image = m_stock_camera;
         image->GetSize(&width, &height);
      }

      float left = (float)(surface_x - 0.5*width);
      float top = (float)(surface_y - 0.5*height);
      float right = (float)(surface_x + 0.5*width);
      float bottom = (float)(surface_y + 0.5*height);

      m_shim->DrawImage(*image, left, top, right, bottom, 0.0f);
   }
}

void kmldrawing::KMLDrawer::RenderNetworkLink(
   const kmldom::NetworkLinkPtr& network_link)
{
   if (!network_link->has_link())
      return;

   // get the href
   std::string href;
   kmldom::LinkPtr link = network_link->get_link();
   GetLinkHref(link, &href);

   // fetch the link and draw

   if (href.length() < 1 || !FetchKML(href, true))
      return;

   RenderElement(m_kml_files.top()->get_root());

   m_kml_files.pop();
   m_kmz_files.pop();
   m_uris.pop();

   // set a timer if the link is to auto refresh

   if (link->has_refreshmode() && link->has_refreshinterval()
      && link->get_refreshmode() == kmldom::REFRESHMODE_ONINTERVAL)
   {
      unsigned long ms = (unsigned long)(1000.0*link->get_refreshinterval());
      if (ms > 0)
         m_shim->SetTimer(0, ms);
   }
}

void kmldrawing::KMLDrawer::SetPen(
   const kmlbase::Color32& color, float width, Pen** pen)
{
   if (m_current_pen == nullptr
      || m_current_pen_width != width
      || color != m_current_pen_color)
   {
      // need to (re)create pen
      m_shim->CreatePen(color, width, &m_current_pen);
      m_current_pen_width = width;
      m_current_pen_color = color;
   }

   *pen = m_current_pen;
}

void kmldrawing::KMLDrawer::SetBrush(
   const kmlbase::Color32& color, Brush** brush)
{
   if (m_current_brush == nullptr || color != m_current_brush_color)
   {
      if (color == s_white)
      {
         // optimization for default Google Earth color
         m_current_brush = m_white_brush;
      }
      else
      {
         // need to (re)create brush
         m_shim->CreateBrush(color, &m_current_brush);
         m_current_brush_color = color;
      }
   }

   *brush = m_current_brush;
}

void kmldrawing::KMLDrawer::FetchImage(const std::string& href, Image** image,
   bool will_be_used_for_geo /*= false*/)
{
   *image = nullptr; // nullptr on failure
   auto it = m_image_cache.find(href);

   if (it == m_image_cache.end())
   {
      // need to create the bitmap
      std::string image_str;
      if (FetchData(href, &image_str))
      {
         m_shim->CreateImageFromRawBytes(
            const_cast<unsigned char*>(
            reinterpret_cast<const unsigned char*>(
            image_str.c_str())), image_str.length(), image,
            will_be_used_for_geo);

         m_image_cache[href] = *image;
      }
   }
   else
      *image = it->second;
}

// stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring

// trim from start
static inline std::string &ltrim(std::string &s)
{
   s.erase(s.begin(), std::find_if(s.begin(), s.end(),
      std::not1(std::ptr_fun<int, int>(std::isspace))));
   return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
   s.erase(std::find_if(s.rbegin(), s.rend(),
      std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
   return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
   return ltrim(rtrim(s));
}

bool kmldrawing::DeconstructUri(
   const std::string& uri, struct DeconstructedUri* deconstructed)
{
   //kmlbase::UriParser* parser
   //   = kmlbase::UriParser::CreateFromParse(href->c_str());

   // We steal the CreateFromParse logic here and recreate it so that we
   // perform both new and delete here.  This prevents heap corruption on
   // Windows where the new and delete operators may be incompatible between
   // different MSVCRT versions.

   kmlbase::UriParser* parser = new kmlbase::UriParser;
   if (!parser->Parse(uri.c_str()))
   {
      delete parser;
      return false;
   }

   parser->GetScheme(&deconstructed->scheme);
   parser->GetHost(&deconstructed->host);
   parser->GetPort(&deconstructed->port);
   parser->GetPath(&deconstructed->path);
   parser->GetQuery(&deconstructed->query);
   parser->GetFragment(&deconstructed->fragment);

   delete parser;

   return true;
}

void kmldrawing::ReconstructUri(
   const struct DeconstructedUri& deconstructed, std::string* uri)
{
   std::stringstream ss;

   if (deconstructed.scheme.length() > 0 && deconstructed.host.length() > 0)
   {
      ss << deconstructed.scheme << "://" << deconstructed.host;
      if (deconstructed.port.length() > 0)
         ss << ':' << deconstructed.port;
      ss << '/';
   }

   ss << deconstructed.path;

   if (deconstructed.query.length() > 0)
      ss << '?' << deconstructed.query;

   if (deconstructed.fragment.length() > 0)
      ss << '#' << deconstructed.fragment;

   *uri = ss.str();
}

void kmldrawing::KMLDrawer::GetLinkHref(
   const kmldom::LinkPtr& link, std::string* href)
{
   if (!link->has_href())
      return;

   *href = link->get_href();
   trim(*href); // (Google Earth behavior)

   // deconstruct the URI components

   DeconstructedUri deconstructed;
   if (!DeconstructUri(*href, &deconstructed))
      return; // may be a straight file name

   if (deconstructed.scheme == "file")
      return;

   // append <httpQuery> tag stuff to query

   std::string httpquery;
   if (link->has_httpquery())
   {
      httpquery = link->get_httpquery();
      trim(httpquery); // (Google Earth behavior)
   }

   if (httpquery.size() > 0)
   {
      std::stringstream ss;

      size_t pos = 0;
      size_t pos_start = httpquery.find('[');
      size_t pos_end = httpquery.find(']', pos_start + 1);

      while (pos_start != std::string::npos)
      {
         // sample:
         // client=Google+Earth&version=7.0.1.8244&kmlversion=2.2&language=en

         ss << httpquery.substr(pos, pos_start - pos);

         std::string tag = httpquery.substr(
            pos_start + 1, pos_end - pos_start - 1);

         if (tag == "clientVersion")
            ss << "5.2.0";
         else if (tag == "kmlVersion")
            ss << "2.2";
         else if (tag == "clientName")
            ss << "FalconView";
         else if (tag == "language")
            ss << "en";
         else
            ss << '0'; // change to zero if we have no idea how to find the value

         pos = pos_end + 1;
         pos_start = httpquery.find('[', pos);
         pos_end = httpquery.find(']', pos_start + 1);
      }

      ss << httpquery.substr(pos);

      if (deconstructed.query.length() > 0)
      {
         // append another ampersand as needed
         if (deconstructed.query[deconstructed.query.length() - 1] != '&')
            deconstructed.query += '&';
      }

      deconstructed.query += ss.str();
   }

   // append <viewFormat> tag stuff to query

   std::string viewformat;

   if (link->has_viewformat())
   {
      viewformat = link->get_viewformat();
      trim(viewformat); // (Google Earth behavior)
   }
   else if (link->has_viewrefreshmode()
      && link->get_viewrefreshmode() == kmldom::VIEWREFRESHMODE_ONSTOP)
   {
      // "If you specify a <viewRefreshMode> of onStop and do not include
      // the <viewFormat> tag in the file, the following information is
      // automatically appended to the query string:
      viewformat = "BBOX=[bboxWest],[bboxSouth],[bboxEast],[bboxNorth]";
   }

   if (viewformat.size() > 0)
   {
      // "If you specify an empty <viewFormat> tag,
      // no information is appended to the query string."

      std::stringstream ss;

      size_t pos = 0;
      size_t pos_start = viewformat.find('[');
      size_t pos_end = viewformat.find(']', pos_start + 1);

      while (pos_start != std::string::npos)
      {
         ss << viewformat.substr(pos, pos_start - pos);

         std::string tag = viewformat.substr(
            pos_start + 1, pos_end - pos_start - 1);

         if (tag == "lookatLon" || tag == "lookatTerrainLon"
            || tag == "cameraLon")
            ss << m_center_lon;
         else if (tag == "lookatLat"
            || tag == "lookatTerrainLat" || tag == "cameraLat")
            ss << m_center_lat;
         else if (tag == "bboxWest")
            ss << m_left_lon;
         else if (tag == "bboxSouth")
            ss << m_bottom_lat;
         else if (tag == "bboxEast")
            ss << m_right_lon;
         else if (tag == "bboxNorth")
            ss << m_top_lat;
         else if (tag == "horizPixels")
            ss << m_surface_width;
         else if (tag == "vertPixels")
            ss << m_surface_height;
         else
            ss << '0'; // change to zero if we have no idea how to find the value

         pos = pos_end + 1;
         pos_start = viewformat.find('[', pos);
         pos_end = viewformat.find(']', pos_start + 1);
      }

      ss << viewformat.substr(pos);

      if (deconstructed.query.length() > 0)
      {
         // append another ampersand as needed
         if (deconstructed.query[deconstructed.query.length() - 1] != '&')
            deconstructed.query += '&';
      }

      deconstructed.query += ss.str();
   }

   ReconstructUri(deconstructed, href);
}

inline bool EndsWithKMZ(const std::string& s)
{
   return s.size() > 2 && strcasecmp(s.c_str() + s.size() - 3, "kmz") == 0;
}

bool kmldrawing::KMLDrawer::FetchData(
   const std::string& uri, std::string* data)
{
   // This fetches data relative to the current KML location.  The uri could be
   // an absolute uri, a file name, a uri relative to the current KML file,
   // a uri that points to KMZ outside of the KML file, etc.  This data fetcher
   // will use the KML cache so that objects are reused.

   // try fetching using KMZ-aware fetching
   if (m_uris.size() > 0
      && EndsWithKMZ(m_uris.top()))
   {
      if (m_kml_cache->FetchDataRelative(m_uris.top() + "/", uri, data))
         return true;

      // If a KMZ has an internal file with a space, LibKML does not resolve
      // the unescaped URI.  This is an ugly work around for this uncommon case.

      kmlengine::KmzFilePtr kmz_file = nullptr;
      auto it = m_kmz_file_cache.find(m_uris.top());

      if (it == m_kmz_file_cache.end())
      {
         // refetch (ouch!) the KMZ

         std::string kmz;
         if (FetchData(m_uris.top(), &kmz)
            && kmlengine::KmzFile::IsKmz(kmz))
         {
            // we have fetched raw KMZ
            kmz_file = kmlengine::KmzFile::OpenFromString(kmz);
         }

         m_kmz_file_cache[m_uris.top()] = kmz_file; // may cache nullptr
      }
      else
      {
         kmz_file = it->second; // may be nullptr
      }

      if (kmz_file != nullptr && kmz_file->ReadFile(uri.c_str(), data))
         return true;
   }

   // try the KmlCache relative to our current URI
   if (m_uris.size() > 0
      && m_kml_cache->FetchDataRelative(
      m_uris.top(), uri, data))
      return true;

   // try to fetch from any kmz file that we have
   if (m_kmz_files.size() > 0
      && m_kmz_files.top() != nullptr
      && m_kmz_files.top()->ReadFile(uri.c_str(), data))
      return true;

   // special handling for possibly local files
   std::ifstream in_file(uri, std::ios_base::in);
   bool is_local_file = in_file.good();
   in_file.close();
   std::string as_uri;
   if (is_local_file
      && kmlbase::UriParser::FilenameToUri(uri, &as_uri)
      && m_kml_cache->FetchDataRelative(as_uri, as_uri, data))
      return true;

   // last ditch is to try the KmlCache with absolute URL
   return m_kml_cache->FetchDataRelative(uri, uri, data);
}

void kmldrawing::KMLDrawer::FetchPossiblyLocalFile(
   const std::string& file_name, kmlengine::KmlFilePtr* kml_file)
{
   *kml_file = nullptr;

   // map is empty when not testing so this is fast
   bool is_local_file = TestDataMap.find(file_name) != TestDataMap.end();

   if (!is_local_file)
   {
      std::ifstream in_file(file_name, std::ios_base::in);
      is_local_file = in_file.good();
      in_file.close();
   }

   std::string as_uri;
   if (is_local_file && kmlbase::UriParser::FilenameToUri(file_name, &as_uri))
      *kml_file = m_kml_cache->FetchKmlRelative(as_uri, as_uri);
}

bool kmldrawing::KMLDrawer::OpenKML(const std::string& uri)
{
   return FetchKML(uri, true);
}

bool kmldrawing::KMLDrawer::FetchKML(const std::string& uri,
   bool push_state, kmlengine::KmlFilePtr* fetched_file /*= nullptr*/)
{
   // This fetches data relative to the current KML location.  The uri could be
   // an absolute uri, a file name, a uri relative to the current KML file,
   // a uri that points to KMZ outside of the KML file, etc.  This KML fetcher
   // will use the KML cache so that objects are reused and so that styles
   // can be sucessfully resolved using the libkml style resolution
   // capabilities.  If the fetch succeeds, the KML file and
   // the URI are all saved so that future calls to this and to FetchData
   // will be resolved relative to what is being drawn at the time of the call.

   // try to parse as KMZ

   m_shim->LogMessage("Trying KMZ...");

   std::string possible_kmz;
   kmlengine::KmzFilePtr kmz_file = nullptr;
   if (FetchData(uri, &possible_kmz) && kmlengine::KmzFile::IsKmz(possible_kmz))
      kmz_file = kmlengine::KmzFile::OpenFromString(possible_kmz);

   // Now use KML cache to parse as KML with resolved styles.  The KML cache
   // shouldn't make a second load of the data, and the NetFetcher has a test
   // to check this.

   kmlengine::KmlFilePtr kml_file = nullptr;
   std::string resolved_uri(uri);

   // Check out KML file cache on the original URI to avoid (maybe) the next
   // (possibly very slow) step.  This is necessary due to a LibKML bug where
   // KML inside of a KMZ is sometimes reparsed every fetch.

   m_shim->LogMessage("Trying KML file cache...");

   auto it = m_kml_file_cache.find(resolved_uri);
   if (it != m_kml_file_cache.end())
   {
      // We already loaded this previously.  We want the previous load of this
      // KML so that any changes imposed by us (tree control states, etc.) are
      // preserved.
      kml_file = it->second;
   }

   // try the KmlCache relative to our URL

   m_shim->LogMessage("Trying relative fetch...");

   if (kml_file == nullptr && m_uris.size() > 0)
   {
      kml_file = m_kml_cache->FetchKmlRelative(m_uris.top(), uri);

      // we need to save the resolved URI so that we can resolved future
      // requests relative to it

      std::string resolved;
      if (kml_file != nullptr
         && kmlengine::ResolveUri(
         m_uris.top(), uri, &resolved))
         resolved_uri = resolved;
   }

   // special handling for possibly local files

   m_shim->LogMessage("Trying local file...");

   if (kml_file == nullptr)
   {
      FetchPossiblyLocalFile(uri, &kml_file);
      std::string as_uri;
      if (kml_file != nullptr
         && kmlbase::UriParser::FilenameToUri(uri, &as_uri))
         resolved_uri = as_uri;
   }

   // last ditch is to try the KmlCache with absolute URL

   m_shim->LogMessage("Trying absolute URL...");

   if (kml_file == nullptr)
      kml_file = m_kml_cache->FetchKmlRelative(uri, uri);

   if (kml_file == nullptr)
      return false;

   // save the KML, KMZ, URI, etc...

   it = m_kml_file_cache.find(resolved_uri);
   if (it == m_kml_file_cache.end())
   {
      // save
      m_kml_file_cache[resolved_uri] = kml_file;
   }
   else
   {
      // We already loaded this previously.  We want the previous load of this
      // KML so that any changes imposed by us (tree control states, etc.) are
      // preserved.
      kml_file = it->second;
   }

   if (push_state)
   {
      m_kml_files.push(kml_file);
      m_kmz_files.push(kmz_file);
      m_uris.push(resolved_uri);
   }

   if (fetched_file != nullptr)
      *fetched_file = kml_file;

   return true;
}

void kmldrawing::KMLDrawer::RemoveTextFromString(
   const std::string& to_remove, std::string* text)
{
   // replace with at single space
   size_t pos = text->find(to_remove);
   if (pos != text->npos)
      text->replace(pos, to_remove.length(), " ");
}

void kmldrawing::KMLDrawer::CreateBalloonText(
   const kmldom::FeaturePtr& feature, std::string* text)
{
   // use LibKML's CreateBalloonText, if we have a KML file to work with
   if (m_kml_files.size() > 0)
      *text = kmlengine::CreateBalloonText(m_kml_files.top(), feature);

   // LibKML doesn't support $[geDirections] (and neither do we)
   RemoveTextFromString("$[geDirections]", text);

   // for photo overlays, try to embed the photo in HTML
   kmldom::PhotoOverlayPtr photo_overlay = kmldom::AsPhotoOverlay(feature);
   if (photo_overlay != nullptr && photo_overlay->has_icon()
      && photo_overlay->get_icon()->has_href() && text->length() == 0)
   {
      std::stringstream ss;

      ss << "<img src=\"" << photo_overlay->get_icon()->get_href()
         << "\">";

      *text = ss.str();
   }

   // if no balloon text, draw data as a table
   if (text->length() == 0 && feature->has_extendeddata())
   {
      std::stringstream ss;

      kmldom::ExtendedDataPtr extendeddata = feature->get_extendeddata();
      size_t size = extendeddata->get_data_array_size();
      for (size_t i = 0; i < size; i++)
      {
         kmldom::DataPtr data = extendeddata->get_data_array_at(i);
         ss << "<b>" << data->get_name() << "</b>: "
            << data->get_value() << "<br>";
      }

      *text = ss.str();
   }

   // save the text if we are in a test environment
   if (kmldrawing::InTestEnvironment)
      s_last_balloon_text = *text;
}

void kmldrawing::KMLDrawer::CreateHoverText(
   const kmldom::FeaturePtr& feature, std::string* text)
{
   *text = feature->get_name();
}

inline double DistanceBetweenPointsSquared(
   double x1, double y1, double x2, double y2)
{
   return (x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2);
}

// (x3, y3) is the lone point
double DistanceBetweenPointAndLineSegmentSquared(
   double x1, double y1, double x2, double y2, double x3, double y3)
{
    double dx = x2 - x1;
    double dy = y2 - y1;
    double m = dx*dx + dy*dy;
    double u = ((x3 - x1)*dx + (y3 - y1)*dy)/m;
    if (u > 1.0)
       return DistanceBetweenPointsSquared(x1 + dx, y1 + dy, x3, y3);
    else if (u < 0.0)
       return DistanceBetweenPointsSquared(x1 + dx, y1 + dy, x3, y3);
    return DistanceBetweenPointsSquared(x1 + u*dx, y1 + u*dy, x3, y3);
}

double DistanceBetweenPointsAndCoordinatesArraySquared(
   kmldom::CoordinatesPtr coordinates, double lat, double lon)
{
   double nearest_distancesq = DBL_MAX;
   size_t size = coordinates->get_coordinates_array_size();

   if (size > 1)
   {
      if (size < 2)
      {
         return DistanceBetweenPointsSquared(
            lon, lat,
            coordinates->get_coordinates_array_at(0).get_longitude(),
            coordinates->get_coordinates_array_at(0).get_latitude());
      }

      for (size_t i = 1; i < size; i++)
      {
         const kmlbase::Vec3 previous
            = coordinates->get_coordinates_array_at(i - 1);
         const kmlbase::Vec3 current
            = coordinates->get_coordinates_array_at(i);
         double distancesq = DistanceBetweenPointAndLineSegmentSquared(
            previous.get_longitude(), previous.get_latitude(),
            current.get_longitude(), current.get_latitude(),
            lon, lat);
         if (distancesq < nearest_distancesq)
            nearest_distancesq = distancesq;
      }
   }

   return nearest_distancesq;
}

double kmldrawing::DistanceFromGeometryToPointSquared(
   const kmldom::GeometryPtr& geometry, double lat, double lon)
{
   kmldom::KmlDomType type = geometry->Type();

   switch (type)
   {
   case kmldom::Type_MultiGeometry:
      {
         double nearest_distancesq = DBL_MAX;
         const kmldom::MultiGeometryPtr multi_geometry
            = kmldom::AsMultiGeometry(geometry);
         size_t size = multi_geometry->get_geometry_array_size();

         for (size_t i = 0; i < size; i++)
         {
            double distancesq = DistanceFromGeometryToPointSquared(
               multi_geometry->get_geometry_array_at(i), lat, lon);
            if (distancesq < nearest_distancesq)
               nearest_distancesq = distancesq;
         }

         return nearest_distancesq;
      }

   case kmldom::Type_Point:
      {
         const kmldom::CoordinatesPtr coordinates
            = kmldom::AsPoint(geometry)->get_coordinates();
         const kmlbase::Vec3 vec = coordinates->get_coordinates_array_at(0);
         return DistanceBetweenPointsSquared(
            lon, lat, vec.get_longitude(), vec.get_latitude());
      }

   case kmldom::Type_LineString:
      {
         kmldom::LineStringPtr line_string = kmldom::AsLineString(geometry);
         if (!line_string->has_coordinates())
            return DBL_MAX; // required element
         return DistanceBetweenPointsAndCoordinatesArraySquared(
            line_string->get_coordinates(), lat, lon);
      }

   case kmldom::Type_Polygon:
      {
         // return the distance from the outer ring for now
         kmldom::PolygonPtr polygon = kmldom::AsPolygon(geometry);
         if (!polygon->has_outerboundaryis()) // required element
            return DBL_MAX;
         kmldom::OuterBoundaryIsPtr outer_boundary
            = polygon->get_outerboundaryis();
         if (!outer_boundary->has_linearring())
            return DBL_MAX;
         kmldom::LinearRingPtr linear_ring = outer_boundary->get_linearring();
         if (!linear_ring->has_coordinates())
            return DBL_MAX; // required element
         return DistanceBetweenPointsAndCoordinatesArraySquared(
            linear_ring->get_coordinates(), lat, lon);
      }

   case kmldom::Type_Model:
      {
         kmldom::ModelPtr model = kmldom::AsModel(geometry);
         if (!model->has_location())
            break;
         const kmldom::LocationPtr location = model->get_location();
         if (!(location->has_latitude() && location->has_longitude()))
            break;
         return DistanceBetweenPointsSquared(
            lon, lat, location->get_longitude(), location->get_latitude());
      }


    default:
      break; // manna?
   }

   return DBL_MAX; // what happened?
}

bool kmldrawing::FeatureAndAncestorsVisible(const kmldom::FeaturePtr& feature)
{
   if (feature == nullptr)
      return true; // recursion terminating condition
   if (!feature->get_visibility())
      return false;
   return FeatureAndAncestorsVisible(kmldom::AsFeature(feature->GetParent()));
}

void kmldrawing::KMLDrawer::HitTest(long x, long y, kmldom::FeaturePtr* feature)
{
   // find the geolocation of the mouse click
   double lat, lon;
   m_shim->SurfaceToGeo(x, y, &lat, &lon);

   // find the feature nearest the mouse click
   kmldom::ObjectPtr obj;
   double nearest_distancesq = m_quad_tree.FindNearest(lat, lon, &obj);
   kmldom::FeaturePtr nearest_feature = kmldom::AsFeature(obj);

   // make sure that the feature really is reasonably close to the click
   if (nearest_feature != nullptr)
   {
      const int EPSILON = 7; // pixels
      double lat2, lon2;
      m_shim->SurfaceToGeo(x + EPSILON, y + EPSILON, &lat2, &lon2);
      double d = DistanceBetweenPointsSquared(lon, lat, lon2, lat2);
      if (nearest_distancesq <= d)
         *feature = nearest_feature;
   }
}

bool kmldrawing::KMLDrawer::HitTestForSelection(long x, long y)
{
   // hit test
   kmldom::FeaturePtr feature = nullptr;
   HitTest(x, y, &feature);
   bool handled = feature != nullptr;

   if (handled)
   {
      m_selected_feature = feature;

      // resolve the information dialog title
      std::string title(
         feature->has_name() ? feature->get_name() : "KML Feature");

      // resolve the information dialog text
      std::string text;
      CreateBalloonText(feature, &text);

      // show the information dialog
      m_shim->ShowInformation(title, text);
      ++s_information_dialog_opens;
   }

   // unselect any selected placemark if we didn't select a new one
   if (!handled && m_selected_feature != nullptr)
   {
      m_selected_feature = nullptr;
      handled = true;
   }

   // handled is false if nothing clicked and no previously selected placemark
   if (handled)
      m_shim->Invalidate(nullptr);

   return handled;
}

void kmldrawing::KMLDrawer::DeselectSelectedFeature()
{
   m_selected_feature = nullptr;
}

//******************************************************************************
//
// BEGIN CODE COPYRIGHTED BY GOOGLE
// See copyright in header file.
// This code has been modified from its original version.
// (see FalconView SVN history)
//
//******************************************************************************
#if 0
// CURLOPT_WRITEFUNCTION:
// size*nmemb bytes of data are at ptr, stream is user data
// return must be size*nmemb or curl itself will fail.
static size_t FetchToString(void* ptr, size_t size, size_t nmemb, void* user) {
   // static function, only user is DoCurlToString which uses CURLOPT_WRITEDATA
   // to set the "user" arg which is the C++ string (buffer) to write to.
   size_t nbytes = size * nmemb;
   string *output_buffer = reinterpret_cast<string*>(user);
   output_buffer->append(reinterpret_cast<char*>(ptr), nbytes);
   return nbytes;
}

// Separate worker function to simplify bool return logic.
static bool DoCurlToString(CURL* curl, const char* url, string* data) {
#define CURLOK(f) (f == CURLE_OK)
   if (CURLOK(curl_easy_setopt(curl, CURLOPT_URL, url)) &&
      CURLOK(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, FetchToString)) &&
      CURLOK(curl_easy_setopt(curl, CURLOPT_WRITEDATA,
      reinterpret_cast<void*>(data))) &&
      CURLOK(curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1)) &&
      CURLOK(curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120)) &&

      /* TODO IMPORTANT: configure certificate validation */
      CURLOK(curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L)) &&
      CURLOK(curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L)) &&

      CURLOK(curl_easy_perform(curl)))
   {
      long http_code = 0;
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
      return http_code == 0 || http_code == 200;
   }
   return false;
}

// Wrapper to manage curl handle.  Very simple stateless implementation.  Less
// simplistic would be to reuse the CURL* handle between invocations.
bool CurlToString(const char* url, string* data) {
   CURL* curl = curl_easy_init();
   bool ret = DoCurlToString(curl, url, data);
   curl_easy_cleanup(curl);
   return ret;
}
#endif
//******************************************************************************
//
// END CODE COPYRIGHTED BY GOOGLE
//
//******************************************************************************

//bool kmldrawing::FetchWithCurl(const std::string& url, std::string* data)
//{
//   CurlNetFetcher net_fetcher;
//   bool fetched = net_fetcher.FetchUrl(url, data);
//
//   if (!fetched)
//   {
//      // LibKML has a buggy behavior where, when fetching KMZ files from the
//      // local file system, it converts the URI back to a file name and then
//      // tries to fetch it.  Curl needs a URI, not a local file name.  Using
//      // only FilenameToUri doesn't always work because LibKML retains
//      // escaped characters.  This work around removes escaped
//      // characters from the file name and then converts back to a URI.
//
//      std::string s1, s2;
//      kmlbase::UriParser::UriToFilename(url, &s1);
//      kmlbase::UriParser::FilenameToUri(s1, &s2);
//      fetched = net_fetcher.FetchUrl(s2, data);
//   }
//
//   return fetched;
//}

bool kmldrawing::KMLNetFetcher::FetchUrl(
   const std::string& url, std::string* data) const
{
   if (kmldrawing::InTestEnvironment)
   {
      // Make sure that we don't fetch the same URL twice.  The cache should
      // ensure this.
      kmldrawing::URLFetchCount[url]++;
   }

   if (data == nullptr)
      return false;

   if (kmldrawing::FetchAsTestData != nullptr
      && kmldrawing::FetchAsTestData(url, data))
      return true;

   // If we have a shim available, use its fetch method.  If no shim has been
   // specified, use the FetchWithCurl conveinence method.  Ideally, a shim
   // should use the FetchWithCurl convience method internally so that tests
   // are true.

   return m_shim == nullptr
      ? false // FetchWithCurl(url, data)
      : m_shim->Fetch(url, data);
}

void kmldrawing::KMLDrawer::TimerFired(int id, void* ptr)
{
   // remember that this may be called from an auxiliary thread
   m_rebuild_cache_on_draw = true;
   m_shim->Invalidate(ptr);
}

bool kmldrawing::ParseKMLDateTime(
      const std::string& date_time, struct tm* tm_val,
      int* offset_hours, int* offset_minutes, bool* negative_offset,
      std::string* error)
{
   //
   // From KML Reference:
   //
   // Specifies a single moment in time. The value is a dateTime, which can be
   // one of the following:
   //
   // dateTime gives second resolution
   // date gives day resolution
   // gYearMonth gives month resolution
   // gYear gives year resolution
   //
   // The following examples show different resolutions for the <when> value:
   // gYear (YYYY)
   // <TimeStamp>
   //   <when>1997</when>
   // </TimeStamp>
   // gYearMonth (YYYY-MM)
   // <TimeStamp>
   //   <when>1997-07</when>
   // </TimeStamp>
   // date (YYYY-MM-DD)
   // <TimeStamp>
   //   <when>1997-07-16</when>
   // </TimeStamp>
   // dateTime (YYYY-MM-DDThh:mm:ssZ)
   // Here, T is the separator between the calendar and the hourly notation
   // of time, and Z indicates UTC. (Seconds are required.)
   // <TimeStamp>
   //   <when>1997-07-16T07:30:15Z</when>
   // </TimeStamp>
   // dateTime (YYYY-MM-DDThh:mm:sszzzzzz)
   // This example gives the local time and then the � conversion to UTC.
   // <TimeStamp>
   //   <when>1997-07-16T10:30:15+03:00</when>
   // </TimeStamp>
   //

   if (tm_val == nullptr || offset_hours == nullptr
      || offset_minutes == nullptr || negative_offset == nullptr)
   {
      *error = "null parameter";
      return false;
   }

   memset(tm_val, 0, sizeof(*tm_val));
   *offset_hours = 0;
   *offset_minutes = 0;
   *negative_offset = false;

   size_t len = date_time.length();

   if (len < 4)
   {
      *error = "datetime length too short";
      return false;
   }

   // parse year
   std::string year_string = date_time.substr(0, 4);
   tm_val->tm_year = atoi(year_string.c_str()) - 1900;
   if (tm_val->tm_year < 0 || tm_val->tm_year > 1000)
   {
      *error = "invalid year";
      return false;
   }

   // parse month
   tm_val->tm_mon = 0;
   if (len > 4)
   {
      if (len < 7)
      {
         *error = "datetime length should not be 5 or 6";
         return false;
      }

      std::string month_string = date_time.substr(5, 2);
      tm_val->tm_mon = atoi(month_string.c_str()) - 1;
      if (tm_val->tm_mon < 0 || tm_val->tm_mon > 11)
      {
         *error = "invalid month";
         return false;
      }
   }

   // parse day
   tm_val->tm_mday = 1;
   if (len > 7)
   {
      if (len < 10)
      {
         *error = "datetime length should not be 8 or 9";
         return false;
      }

      std::string day_string = date_time.substr(8, 2);
      tm_val->tm_mday = atoi(day_string.c_str());
      if (tm_val->tm_mday < 1 || tm_val->tm_mday > 31)
      {
         *error = "invalid day";
         return false;
      }
   }

   // parse hour, minute, second and time zone offset
   if (len > 10)
   {
      // we allow a length of 19 to allow for times that omit the Z for Zulu
      if (len != 19 && len != 20 && len != 25)
      {
         *error = "invalid datetime";
         return false;
      }

      std::string hour_string = date_time.substr(11, 2);
      tm_val->tm_hour = atoi(hour_string.c_str());
      if (tm_val->tm_hour < 0 || tm_val->tm_hour > 23)
      {
         *error = "invalid hour";
         return false;
      }

      std::string minute_string = date_time.substr(14, 2);
      tm_val->tm_min = atoi(minute_string.c_str());
      if (tm_val->tm_min < 0 || tm_val->tm_min > 59)
      {
         *error = "invalid minute";
         return false;
      }

      std::string second_string = date_time.substr(17, 2);
      tm_val->tm_sec = atoi(second_string.c_str());
      if (tm_val->tm_sec < 0 || tm_val->tm_sec > 59)
      {
         *error = "invalid second";
         return false;
      }

      if (len == 25) // zone offset, not Zulu
      {
         std::string offset_hours_string = date_time.substr(20, 2);
         *offset_hours = atoi(offset_hours_string.c_str());
         if (*offset_hours < 0 || *offset_hours > 13)
         {
            *error = "invalid offset hours";
            return false;
         }

         std::string offset_minutes_string = date_time.substr(22, 2);
         *offset_minutes = atoi(offset_minutes_string.c_str());
         if (*offset_minutes < 0 || *offset_minutes > 59)
         {
            *error = "invalid offset minutes";
            return false;
         }

         *negative_offset = date_time[19] == '-';
      }
   }

   return true;
}

// returns false on failure
bool kmldrawing::GetBboxFromGeometry(const kmldom::GeometryPtr& geometry,
   kmlengine::Bbox* bbox)
{
   if (geometry == nullptr)
      return false;

   kmldom::KmlDomType type = geometry->Type();

   switch (type)
   {
   case kmldom::Type_MultiGeometry:
      {
         const kmldom::MultiGeometryPtr multi_geometry
            = kmldom::AsMultiGeometry(geometry);
         size_t size = multi_geometry->get_geometry_array_size();
         bool bbox_initialized = false;

         for (size_t i = 0; i < size; i++)
         {
            if (!bbox_initialized)
            {
               bbox_initialized = GetBboxFromGeometry(
                  multi_geometry->get_geometry_array_at(i), bbox);
            }
            else
            {
               kmlengine::Bbox sub_bbox;
               GetBboxFromGeometry(
                  multi_geometry->get_geometry_array_at(i), &sub_bbox);
               bbox->ExpandFromBbox(sub_bbox);
            }
         }

         return bbox_initialized;
      }

   case kmldom::Type_Point:
      {
         const kmldom::PointPtr point = kmldom::AsPoint(geometry);
         if (!point->has_coordinates())
            return false;
         const kmldom::CoordinatesPtr coordinates = point->get_coordinates();
         if (coordinates->get_coordinates_array_size() < 1)
            return false;
         const kmlbase::Vec3 vec = coordinates->get_coordinates_array_at(0);

         double lat = vec.get_latitude();
         double lon = vec.get_longitude();

         bbox->set_north(lat);
         bbox->set_south(lat);
         bbox->set_east(lon);
         bbox->set_west(lon);

         return true;
      }

   case kmldom::Type_LineString:
      {
         const kmldom::LineStringPtr line_string
            = kmldom::AsLineString(geometry);
         if (!line_string->has_coordinates())
            return false; // required element
         const kmldom::CoordinatesPtr coordinates
            = line_string->get_coordinates();
         size_t size = coordinates->get_coordinates_array_size();
         bool bbox_initialized = false;

         for (size_t i = 0; i < size; i++)
         {
            if (!bbox_initialized)
            {
               const kmlbase::Vec3 vec
                  = coordinates->get_coordinates_array_at(i);

               double lat = vec.get_latitude();
               double lon = vec.get_longitude();

               bbox->set_north(lat);
               bbox->set_south(lat);
               bbox->set_east(lon);
               bbox->set_west(lon);

               bbox_initialized = true;
            }
            else
            {
               const kmlbase::Vec3 vec
                  = coordinates->get_coordinates_array_at(i);
               bbox->ExpandLatLon(vec.get_latitude(), vec.get_longitude());
            }
         }

         return bbox_initialized;
      }

   case kmldom::Type_Polygon:
      {
         kmldom::PolygonPtr polygon = kmldom::AsPolygon(geometry);
         if (!polygon->has_outerboundaryis()) // required element
            return false;
         const kmldom::OuterBoundaryIsPtr outer_boundary
            = polygon->get_outerboundaryis();
         if (!outer_boundary->has_linearring())
            return false;
         const kmldom::LinearRingPtr linear_ring
            = outer_boundary->get_linearring();
         if (!linear_ring->has_coordinates())
            return false; // required element
         const kmldom::CoordinatesPtr coordinates
            = linear_ring->get_coordinates();
         size_t size = coordinates->get_coordinates_array_size();
         bool bbox_initialized = false;

         for (size_t i = 0; i < size; i++)
         {
            if (!bbox_initialized)
            {
               const kmlbase::Vec3 vec
                  = coordinates->get_coordinates_array_at(i);

               double lat = vec.get_latitude();
               double lon = vec.get_longitude();

               bbox->set_north(lat);
               bbox->set_south(lat);
               bbox->set_east(lon);
               bbox->set_west(lon);

               bbox_initialized = true;
            }
            else
            {
               const kmlbase::Vec3 vec
                  = coordinates->get_coordinates_array_at(i);
               bbox->ExpandLatLon(vec.get_latitude(), vec.get_longitude());
            }
         }

         return bbox_initialized;
      }

   case kmldom::Type_Model:
      {
         kmldom::ModelPtr model = kmldom::AsModel(geometry);
         if (!model->has_location())
            return false;
         const kmldom::LocationPtr location = model->get_location();
         if (!(location->has_latitude() && location->has_longitude()))
            return false;

         double lat = location->get_latitude();
         double lon = location->get_longitude();

         bbox->set_north(lat);
         bbox->set_south(lat);
         bbox->set_east(lon);
         bbox->set_west(lon);

         return true;
      }

    default:
      KML_DRAWING_ASSERT(false); // manna?
   }

   return false;
}

void kmldrawing::QuadTile::AddObject(const kmldom::ObjectPtr& object)
{
   // make sure that this is a supported object and determine its bounding box

   kmlengine::Bbox bbox;

   kmldom::PlacemarkPtr placemark = kmldom::AsPlacemark(object);

   if (placemark != nullptr)
   {
      if (!placemark->has_geometry()
         || !GetBboxFromGeometry(placemark->get_geometry(), &bbox))
         return;
   }
   else
   {
      kmldom::PhotoOverlayPtr photo_overlay = kmldom::AsPhotoOverlay(object);
      if (photo_overlay == nullptr || !photo_overlay->has_point()
         || !GetBboxFromGeometry(photo_overlay->get_point(), &bbox))
         return;
   }

   // add the object to the tree
   AddObjectKnownBbox(object, bbox);
}

void kmldrawing::QuadTile::AddObjectKnownBbox(const kmldom::ObjectPtr& object,
   const kmlengine::Bbox& bbox)
{
   // if we are at the max tile depth, add and return
   if (m_depth >= QUAD_TILE_MAX_DEPTH)
   {
      m_objs.push_back(object);
      return;
   }

   // determine the sub tile in which this should be placed or place
   // the object in this tile

   double mid_lat = m_bottom_lat + (m_top_lat - m_bottom_lat)/2.0;
   double mid_lon = m_left_lon + (m_right_lon - m_left_lon)/2.0;

   double south = bbox.get_south();
   double west = bbox.get_west();
   double east = bbox.get_east();
   double north = bbox.get_north();

   if (east == west && north == south)
   {
      // shortcut for points

      if (north > mid_lat && east < mid_lon)
      {
         if (m_ul == nullptr)
         {
            m_ul = new QuadTile(
               m_left_lon, mid_lon, m_top_lat, mid_lat, m_depth + 1);
         }

         m_ul->AddObjectKnownBbox(object, bbox);
      }
      else if (north > mid_lat && east > mid_lon)
      {
         if (m_ur == nullptr)
         {
            m_ur = new QuadTile(
               mid_lon, m_right_lon, m_top_lat, mid_lat, m_depth + 1);
         }

         m_ur->AddObjectKnownBbox(object, bbox);
      }
      else if (north < mid_lat && east < mid_lon)
      {
         if (m_ll == nullptr)
         {
            m_ll = new QuadTile(
               m_left_lon, mid_lon, mid_lat, m_bottom_lat, m_depth + 1);
         }

         m_ll->AddObjectKnownBbox(object, bbox);
      }
      else if (north < mid_lat && east > mid_lon)
      {
         if (m_lr == nullptr)
         {
            m_lr = new QuadTile(
               mid_lon, m_right_lon, mid_lat, m_bottom_lat, m_depth + 1);
         }

         m_lr->AddObjectKnownBbox(object, bbox);
      }
      else
      {
         // the point is exactly on a boundary
         m_objs.push_back(object);
      }

      return;
   }

   // not a point

   bool intersects_ul = kmldrawing::DoGeoRectsIntersect(
      south, west, north, east,
      mid_lat, m_left_lon, m_top_lat, mid_lon);

   bool intersects_ur = kmldrawing::DoGeoRectsIntersect(
      south, west, north, east,
      mid_lat, mid_lon, m_top_lat, m_right_lon);

   if (intersects_ul && intersects_ur)
   {
      m_objs.push_back(object);
      return;
   }

   bool intersects_ll = kmldrawing::DoGeoRectsIntersect(
      south, west, north, east,
      m_bottom_lat, m_left_lon, mid_lat, mid_lon);

   if (  (intersects_ul && intersects_ll)
      || (intersects_ur && intersects_ll) )
   {
      m_objs.push_back(object);
      return;
   }

   bool intersects_lr = kmldrawing::DoGeoRectsIntersect(
      south, west, north, east,
      m_bottom_lat, mid_lon, mid_lat, m_right_lon);

   if (  (intersects_ul && intersects_lr)
      || (intersects_ur && intersects_lr)
      || (intersects_ll && intersects_lr) )
   {
      m_objs.push_back(object);
      return;
   }

   // at this point we know that the object should be placed in a sub tile

   if (intersects_ul)
   {
      if (m_ul == nullptr)
      {
         m_ul = new QuadTile(
            m_left_lon, mid_lon, m_top_lat, mid_lat, m_depth + 1);
      }

      m_ul->AddObjectKnownBbox(object, bbox);
   }
   else if (intersects_ur)
   {
      if (m_ur == nullptr)
      {
         m_ur = new QuadTile(
            mid_lon, m_right_lon, m_top_lat, mid_lat, m_depth + 1);
      }

      m_ur->AddObjectKnownBbox(object, bbox);
   }
   else if (intersects_ll)
   {
      if (m_ll == nullptr)
      {
         m_ll = new QuadTile(
            m_left_lon, mid_lon, mid_lat, m_bottom_lat, m_depth + 1);
      }

      m_ll->AddObjectKnownBbox(object, bbox);
   }
   else // intersects lower right
   {
      if (m_lr == nullptr)
      {
         m_lr = new QuadTile(
            mid_lon, m_right_lon, mid_lat, m_bottom_lat, m_depth + 1);
      }

      m_lr->AddObjectKnownBbox(object, bbox);
   }
}

double kmldrawing::QuadTile::CheckTileForCloser(
   const QuadTile* tile_to_check, double current_distancesq,
   double lat, double lon, kmldom::ObjectPtr* nearest) const
{
   if (tile_to_check != nullptr)
   {
      kmldom::ObjectPtr sub_nearest;
      double sub_distancesq = tile_to_check->FindNearest(
         lat, lon, &sub_nearest);

      if (sub_distancesq < current_distancesq)
      {
         // found a closer object
         *nearest = sub_nearest;
         return sub_distancesq;
      }
   }

   return current_distancesq; // nothing closer found
}

double kmldrawing::QuadTile::FindNearest(
   double lat, double lon, kmldom::ObjectPtr* nearest) const
{
   // these are the return values if nothing found
   double nearest_distancesq = DBL_MAX;
   *nearest = nullptr;

   // find the nearest object in this tile

   for (auto it = m_objs.begin(); it != m_objs.end(); it++)
   {
      // try object as a placemark

      kmldom::PlacemarkPtr placemark = kmldom::AsPlacemark(*it);

      if (placemark != nullptr
         && FeatureAndAncestorsVisible(placemark) && placemark->has_geometry())
      {
         double distancesq = DistanceFromGeometryToPointSquared(
            placemark->get_geometry(), lat, lon);

         if (distancesq < nearest_distancesq)
         {
            *nearest = placemark;
            nearest_distancesq = distancesq;
         }
      }

      // try object as a photo overlay

      kmldom::PhotoOverlayPtr photo_overlay
         = kmldom::AsPhotoOverlay(*it);

      if (photo_overlay != nullptr
         && photo_overlay->get_visibility() && photo_overlay->has_point())
      {
         double distancesq = DistanceFromGeometryToPointSquared(
            photo_overlay->get_point(), lat, lon);

         if (distancesq < nearest_distancesq)
         {
            *nearest = photo_overlay;
            nearest_distancesq = distancesq;
         }
      }
   }

   // drill down through sub tiles looking for something even closer

   double mid_lat = m_bottom_lat + (m_top_lat - m_bottom_lat)/2.0;
   double mid_lon = m_left_lon + (m_right_lon - m_left_lon)/2.0;
   double dx2, dy2, dd2; // distances squared (recycled below)

   // check the sub tile that the point actually falls in and possibly
   // check other closer tiles

   if (lat > mid_lat && lon < mid_lon)
   {
      // check upper left
      nearest_distancesq = CheckTileForCloser(m_ul, nearest_distancesq,
         lat, lon, nearest);

      // possibly check upper right
      dx2 = mid_lon - lon;
      dx2 *= dx2;
      if (dx2 < nearest_distancesq)
         nearest_distancesq = CheckTileForCloser(m_ur, nearest_distancesq,
         lat, lon, nearest);

      // possibly check lower left
      dy2 = lat - mid_lat;
      dy2 *= dy2;
      if (dy2 < nearest_distancesq)
         nearest_distancesq = CheckTileForCloser(m_ll, nearest_distancesq,
         lat, lon, nearest);

      // possibly check lower right
      dd2 = dx2 + dy2;
      if (dd2 < nearest_distancesq)
         nearest_distancesq = CheckTileForCloser(m_lr, nearest_distancesq,
         lat, lon, nearest);
   }
   else if (lat > mid_lat && lon > mid_lon)
   {
      // check upper right
      nearest_distancesq = CheckTileForCloser(m_ur, nearest_distancesq,
         lat, lon, nearest);

      // possibly check upper left
      dx2 = lon - mid_lon;
      dx2 *= dx2;
      if (dx2 < nearest_distancesq)
         nearest_distancesq = CheckTileForCloser(m_ul, nearest_distancesq,
         lat, lon, nearest);

      // possibly check lower right
      dy2 = lat - mid_lat;
      dy2 *= dy2;
      if (dy2 < nearest_distancesq)
         nearest_distancesq = CheckTileForCloser(m_lr, nearest_distancesq,
         lat, lon, nearest);

      // possibly check lower left
      dd2 = dx2 + dy2;
      if (dd2 < nearest_distancesq)
         nearest_distancesq = CheckTileForCloser(m_ll, nearest_distancesq,
         lat, lon, nearest);
   }
   else if (lat < mid_lat && lon < mid_lon)
   {
      // check lower left
      nearest_distancesq = CheckTileForCloser(m_ll, nearest_distancesq,
         lat, lon, nearest);

      // possibly check lower right
      dx2 = mid_lon - lon;
      dx2 *= dx2;
      if (dx2 < nearest_distancesq)
         nearest_distancesq = CheckTileForCloser(m_lr, nearest_distancesq,
         lat, lon, nearest);

      // possibly check upper left
      dy2 = mid_lat - lat;
      dy2 *= dy2;
      if (dy2 < nearest_distancesq)
         nearest_distancesq = CheckTileForCloser(m_ul, nearest_distancesq,
         lat, lon, nearest);

      // possibly check upper right
      dd2 = dx2 + dy2;
      if (dd2 < nearest_distancesq)
         nearest_distancesq = CheckTileForCloser(m_ur, nearest_distancesq,
         lat, lon, nearest);
   }
   else
   {
      // check lower right
      nearest_distancesq = CheckTileForCloser(m_lr, nearest_distancesq,
         lat, lon, nearest);

      // possibly check lower left
      dx2 = lon - mid_lon;
      dx2 *= dx2;
      if (dx2 < nearest_distancesq)
         nearest_distancesq = CheckTileForCloser(m_ll, nearest_distancesq,
         lat, lon, nearest);

      // possibly check upper right
      dy2 = mid_lat - lat;
      dy2 *= dy2;
      if (dy2 < nearest_distancesq)
         nearest_distancesq = CheckTileForCloser(m_ur, nearest_distancesq,
         lat, lon, nearest);

      // possibly check upper left
      dd2 = dx2 + dy2;
      if (dd2 < nearest_distancesq)
         nearest_distancesq = CheckTileForCloser(m_ul, nearest_distancesq,
         lat, lon, nearest);
   }

   return nearest_distancesq;
}

void kmldrawing::QuadTile::Clear()
{
   if (m_ul != nullptr)
      m_ul->Clear();
   if (m_ur != nullptr)
      m_ur->Clear();
   if (m_ll != nullptr)
      m_ll->Clear();
   if (m_lr != nullptr)
      m_lr->Clear();

   m_objs.clear();
}
