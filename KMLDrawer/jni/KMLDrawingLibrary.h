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

#ifndef KML_DRAWING_LIBRARY_H_
#define KML_DRAWING_LIBRARY_H_

#define CURRENTLY_DRAWING_ID_LEN 128

#include "kml/engine.h"

namespace kmldrawing
{
   void KML_DRAWING_ASSERT(bool);

   // Pens, brushes and images are drawing objects that must implement the
   // methods below.

   class Pen
   {
   }; // class Pen

   class Brush
   {
   }; // class Brush

   class Image
   {
   public:
      virtual void GetSize(int* width, int* height) const = 0;
   }; // class Image

   // The drawing library calls the appropriate methods on the shim
   // whenever it needs to interact with the application using the library.
   // Setting up the shim is the main task of any user of the drawing library.

   class KMLShim
   {
   public:
      //
      // These methods are for monitoring the drawing process.
      // (FalconView uses this to build a tree control)
      //

      // Call this before and after starting main draw loop.
      virtual void BeginDraw() = 0;
      virtual void EndDraw() = 0;

      // These may be called outside of the main draw loop.  For example,
      // selected elements may receive a second Begin/EndDrawElement outside
      // of the main draw loop.
      virtual void BeginDrawElement(const kmldom::ElementPtr& element) = 0;
      virtual void EndDrawElement(const kmldom::ElementPtr& element) = 0;

      //
      // These methods are for supplying information about the map on which
      // the library is drawing.  Note that we may only draw to an
      // unprojected map.  Any projection must be performed by the library user.
      //

      virtual void GetSurfaceSize(int* width, int* height) = 0;
      virtual void GetMapBounds(double* bottom_lat, double* left_lon,
         double* top_lat, double* right_lon) = 0;
      virtual void GetDegreesPerPixel(double* dpp_y, double* dpp_x) = 0;
      virtual void GeoToSurface(double lat, double lon, int* x, int* y) = 0;
      virtual void SurfaceToGeo(int x, int y, double* lat, double* lon) = 0;

      //
      // These methods are for creating instances of drawing objects.  After
      // a draw is completed, the caller should delete any objects that need
      // to be cleaned up EXCEPT FOR IMAGES.  Images may be reused so that they
      // aren't fetched / recreated repeatedly.  If the drawing library
      // anticipates using an image for DrawImageGeo, it will indicate this
      // when creating the image.
      //

      virtual void CreatePen(
         const kmlbase::Color32& color, float width, Pen** pen) = 0;
      virtual void CreateBrush(
         const kmlbase::Color32& color, Brush** brush) = 0;
      virtual void CreateImageFromRawBytes(
         unsigned char* bytes, size_t length, Image** image,
         bool will_be_used_for_geo = false) = 0;

      //
      // These methods are called when objects need to be drawn.
      // Coordinates are in screen / canvas coordinates.  The origin is the
      // lower left corner of the screen.
      //

      virtual void DrawImage(const Image& image,
         float left, float top, float right, float bottom, float rotation) = 0;
      virtual void DrawImageGeo(const Image& image,
         double west, double north, double east, double south) = 0;
      virtual void DrawEllipse(float x, float y, float x_axis, float y_axis,
         const Pen& pen, const Brush& brush) = 0;
      virtual void DrawPolyPolygon(
         const std::vector<std::pair<float, float>>& points,
         const std::vector<int>& point_counts,
         const Pen& pen, const Brush& brush) = 0;
      virtual void DrawLines(const std::vector<std::pair<float, float>>& points,
         const Pen& pen) = 0;
      virtual void DrawString(const std::string& text, float x, float y,
         float size, const Pen& pen) = 0;

      //
      // This method is to open an information dialog, if supported.  This may
      // be called within a hit test.
      //

      virtual void ShowInformation(
         const std::string& title, const std::string& text) = 0;

      //
      // This method sets a timer on the shim.  When the timer expires, the
      // shim should call TimerFired back on the drawer.  The shim may call
      // TimerFired from an auxiliary thread.
      //

      virtual void SetTimer(int id, unsigned long ms) = 0;

      //
      // The drawing library normally only clears timers when a new draw is
      // started.  The drawing library does not clear timers in its destructor,
      // so the library user should make sure that no timers are fired
      // after object destruction.
      //

      virtual void ClearAllTimers() = 0;

      //
      // Invalidate is called if something has changed that requires a redraw.
      // If this is called as a result of a call to TimerFired, the same void*
      // will be passed back to the shim here, otherwise the void* will be null.
      //

      virtual void Invalidate(void* ptr) = 0;

      //
      // Fetch should return false if a fetch fails.  Fetch may also operate
      // asynchronously by returning false when a fetch is in progress but is
      // not completed.  In that case, the shim should redraw when the fetch
      // is completed, causing the drawer to eventually call Fetch again with
      // the same URI.  In this asynchronous mode, the shim should clear any
      // cache it has of the data as the drawer will cache as appropriate.
      //

      virtual bool Fetch(const std::string& uri, std::string* data) = 0;

      //
      // When the drawer encounters KML with a time primitive, it will query
      // the shim as to whether or not it should draw based on the time
      // primitive.  ParseKMLDateTime is provided as a convenience function
      // for parsing the strings in the time primitive.
      //

      virtual bool ShouldRender(
         const kmldom::TimePrimitivePtr& time_primitive) = 0;
   }; // class KMLShim

   class KMLNetFetcher : public kmlbase::NetFetcher
   {
   public:
      KMLNetFetcher() : m_shim(nullptr) {}
      KMLNetFetcher(KMLShim* shim) : m_shim(shim) {}

   private:
      virtual bool FetchUrl(const std::string& url, std::string* data) const;
      KMLShim* m_shim;
   };

#define QUAD_TILE_MAX_DEPTH 20

   // Quad tiles are used internally to the drawing library for fast
   // hit testing.
   class QuadTile
   {
   public:
      QuadTile(double left_lon = -180.0, double right_lon = 180.0,
         double top_lat = 90.0, double bottom_lat = -90.0, size_t depth = 0)
         : m_left_lon(left_lon), m_right_lon(right_lon),
         m_top_lat(top_lat), m_bottom_lat(bottom_lat),
         m_ul(nullptr), m_ur(nullptr), m_ll(nullptr), m_lr(nullptr),
         m_depth(depth)
      {
      }

      ~QuadTile()
      {
         delete m_ul;
         delete m_ur;
         delete m_ll;
         delete m_lr;
      }

      // Pass in the object.  AddObject will decide if this is a supported type
      // and will calculate the bounding box for placement in the quad tree.
      // This should normally only be called on the top tile in the quad tree.
      void AddObject(const kmldom::ObjectPtr& object);

      // Finds the object nearest the lat, lon.  This should normally only be
      // called on the top tile in the quad tree.  It returns the square of
      // the Euclidean distance to the nearest feature.
      double FindNearest(
         double lat, double lon, kmldom::ObjectPtr* nearest) const;

      // Clears the contents of the quad tile and all of its children.
      void Clear();

   private:
      // Once the object is known to be a type that we support and the Bbox
      // is calculated, we use this function internally.
      void AddObjectKnownBbox(const kmldom::ObjectPtr& object,
         const kmlengine::Bbox& bbox);

      // If tile to check is not null, and if tile to check has an object
      // closer than the current distance, this will replace nearest with 
      // the closer object and return the distance to that object.  If
      // tile to check is null or has no closer object, this returns the
      // current distance (squared) and leaves nearest unchanged.
      double CheckTileForCloser(const QuadTile* tile_to_check,
         double current_distancesq, double lat, double lon,
         kmldom::ObjectPtr* nearest) const;

      // objs contains all of the objects whose bounding boxes are fully
      // contained by this tile, but whose bounding boxes overlap this tile's
      // sub tiles.  The exception is that we will not create new sub tiles
      // if we are already at QUAD_TILE_MAX_DEPTH.
      std::vector<kmldom::ObjectPtr> m_objs;

      // These are the boundaries of this tile.
      double m_left_lon, m_right_lon, m_top_lat, m_bottom_lat;

      // These are the sub tiles of this tile. If they are null, they are
      // empty.
      QuadTile *m_ul, *m_ur, *m_ll, *m_lr;

      // This is the depth of this tile.
      size_t m_depth;
   };

   class KMLDrawer
   {
   public:
      KMLDrawer(KMLShim* shim) : m_shim(shim), m_selected_feature(nullptr),
         m_rebuild_cache_on_draw(false), m_net_fetcher(shim)
      {
         s_currently_drawing_id[0] = '\0';

         // create the KML cache
         // (cache may hold 1,024 items)
         m_kml_cache = new kmlengine::KmlCache(&m_net_fetcher, 1024);
      }

      ~KMLDrawer()
      {
         // clean up
         delete m_kml_cache;
      }

      // Call this method to open a URI referencing some KML.  This must
      // happen before drawing.

      bool OpenKML(const std::string& uri);

      // Call this method to draw.  The drawing library will call back into
      // the shim as it performs the drawing logic.

      void Draw();

      // Hit testing is performed in screen coordinates.  HitTestForSelection
      // returns true if the hit was handled, which generally means that a
      // feature was selected or unselected.

      void HitTest(long x, long y, kmldom::FeaturePtr* feature);
      bool HitTestForSelection(long x, long y);
      void DeselectSelectedFeature();

      // The drawer may set a timer on the shim via SetTimer.  The shim should
      // call this method when the timer is fired.  The drawer expects that
      // this may be called from an auxiliary thread.  The void pointer will
      // be passed back to Invalidate on the shim if this timer results in
      // a redraw.

      void TimerFired(int id, void* ptr);

      // These are static variables are for testing.
      static size_t s_selected_placemark_draws, s_information_dialog_opens,
         s_explicit_uri_styles_resolved, s_colored_placemark_draws;
      static std::string s_last_balloon_text;

      // This creates text that may be displayed when the mouse hovers
      // over a feature.  If nothing should be displayed, text will be "".
      void CreateHoverText(
         const kmldom::FeaturePtr& feature, std::string* text);

      // Some literal bytes for drawing a camera.
      static unsigned char* s_camera_bytes;
      static size_t s_camera_bytes_len;
      Image* m_stock_camera;

   private:
      // These methods are used during the draw.
      void RenderElement(const kmldom::ElementPtr& element,
         bool add_to_quad_tree = true);
      void RenderFeature(const kmldom::FeaturePtr& feature);
      void RenderPlacemark(const kmldom::PlacemarkPtr& placemark);
      void RenderGroundOverlay(const kmldom::GroundOverlayPtr& ground_overlay);
      void RenderGeometry(const kmldom::GeometryPtr& geometry,
         const kmldom::PlacemarkPtr& associated_placemark);
      void RenderPoint(const kmldom::PointPtr& point,
         const kmldom::PlacemarkPtr& associated_placemark);
      void RenderPolygon(const kmldom::PolygonPtr& polygon,
         const kmldom::PlacemarkPtr& associated_placemark);
      void RenderLineString(const kmldom::LineStringPtr& line_string,
         const kmldom::PlacemarkPtr& associated_placemark);
      void RenderScreenOverlay(const kmldom::ScreenOverlayPtr& screen_overlay);
      void RenderPhotoOverlay(const kmldom::PhotoOverlayPtr& photo_overlay);
      void RenderNetworkLink(const kmldom::NetworkLinkPtr& network_link);

      // These are some utility methods.
      bool KMLFeatureInRegion(kmldom::FeaturePtr feature);
      kmldom::ElementPtr GetAncestorOfType(
         const kmldom::ElementPtr& element, kmldom::KmlDomType type);
      kmldom::StylePtr ResolveStyle(const kmldom::PlacemarkPtr& placemark);
      bool FetchKML(const std::string& uri, bool push_state,
         kmlengine::KmlFilePtr* fetched_file = nullptr);
      void FetchImage(const std::string& href, Image** image,
         bool will_be_used_for_geo = false);
      bool FetchData(const std::string& uri, std::string* data);
      void FetchPossiblyLocalFile(
         const std::string& file_name, kmlengine::KmlFilePtr* kml_file);
      void SetPenFromKMLLineStyle(kmldom::StylePtr style, Pen** current_pen);
      const kmldom::AbstractLatLonBoxPtr GetLatLonBox(
         const kmldom::GroundOverlayPtr& groundoverlay);
      void GetLinkHref(const kmldom::LinkPtr& link, std::string* href);
      void CreateBalloonText(
         const kmldom::FeaturePtr& feature, std::string* text);
      void RemoveTextFromString(
         const std::string& to_remove, std::string* text);
      void AddPointToVector(const kmlbase::Vec3& vec,
         std::vector<std::pair<float, float>>* points);

      // These methods set the current pen or brush based on the input
      // parameters.  The output parameter of each method is a pointer to the
      // current pen or brush.
      void SetPen(const kmlbase::Color32& color, float width, Pen** pen);
      void SetBrush(const kmlbase::Color32& color, Brush** brush);

      // The shim must be set before anything else can happen.
      KMLShim* m_shim;

      // These three stacks are used together to push the KML that is being
      // drawn so that relative links, etc. can be resolved in relation to the
      // KML currently being drawn.
      std::stack<kmlengine::KmlFilePtr> m_kml_files;
      std::stack<kmlengine::KmzFilePtr> m_kmz_files;
      std::stack<std::string> m_uris;

      // KML fetched from a cached KMZ sometimes rebuilds the KmlFile each
      // time, so user interface states in the tree control are lost.  We use
      // this to cache the pointers to KML files.
      std::map<std::string, kmlengine::KmlFilePtr> m_kml_file_cache;

      // This cache holds raw KMZ data for cases where LibKML fails to pull
      // files from its internal KMZ cache.  A test case that demonstrates
      // this is test_kmz_internal_space.
      std::map<std::string, kmlengine::KmzFilePtr> m_kmz_file_cache;

      // The quad tree is used for fast hit testing
      QuadTile m_quad_tree;

      // This is the selected feature (if any)
      kmldom::FeaturePtr m_selected_feature;

      // This is used to set whether or not a selected object should be drawn
      // during the draw loop.  This allows us to draw the selected object last.
      bool m_draw_selected;

      // These are saved parameters about the map on which we are drawing.
      int m_surface_width, m_surface_height;
      double m_left_lon, m_right_lon, m_bottom_lat, m_top_lat,
         m_center_lat, m_center_lon, m_dpp_x, m_dpp_y;

      // These are stock objects, kept around for efficiency in drawing.
      static kmlbase::Color32 s_white, s_cyan, s_clear;
      Pen* m_white_pen;
      Brush *m_white_brush, *m_clear_brush;

      // These drawing objects are for reuse so that we don't recreate the
      // same object if styles don't change from placemark to placemark.

      kmlbase::Color32 m_current_pen_color, m_current_brush_color;
      float m_current_pen_width;
      Pen* m_current_pen;
      Brush* m_current_brush;

      // In cases where the KML automatically refreshes, the cache must be
      // rebuilt each time.
      bool m_rebuild_cache_on_draw;

      // Cache has problem.  Suppose two different KMZ files, both
      // linked via a "parent" KML both have "icon.png."  Need to
      // create test case where icon.png is different for both files and
      // adjust cache to remember where it was loaded from.  Maybe hash
      // the fetched icon data (fetched from cache hopefully) to avoid
      // recreating the image.
      // See http://stackoverflow.com/questions/8029121/how-to-hash-stdstring
      // for ideas.
      std::map<std::string, Image*> m_image_cache;

      // All data is fetched through a master cache.  This allows shared style
      // resolution, among other benefits.
      KMLNetFetcher m_net_fetcher;
      kmlengine::KmlCache* m_kml_cache;

      // A buffer that is only used during non-threaded debugging.
      static char s_currently_drawing_id[CURRENTLY_DRAWING_ID_LEN];
   }; // class KMLDrawer

   struct DeconstructedUri
   {
      std::string scheme, host, port, path, query, fragment;
   };

   // These are some utility functions used within the library.
   extern double DistanceFromGeometryToPointSquared(
      const kmldom::GeometryPtr& geometry, double lat, double lon);
   extern bool StyleAppearsResolved(const kmldom::StylePtr& style);
   extern bool FeatureAndAncestorsVisible(const kmldom::FeaturePtr& feature);
   extern bool DeconstructUri(
      const std::string& uri, struct DeconstructedUri* deconstructed);
   extern bool LongitudeInRange(
      double western_lon, double eastern_lon, double point_lon);
   extern bool DoGeoRectsIntersect(double ll_A_lat, double ll_A_lon,
      double ur_A_lat, double ur_A_lon,
      double ll_B_lat, double ll_B_lon,
      double ur_B_lat, double ur_B_lon);
   extern void ReconstructUri(const struct DeconstructedUri& deconstructed,
      std::string* uri);
   extern bool GetBboxFromGeometry(const kmldom::GeometryPtr& geometry,
      kmlengine::Bbox* bbox);

   // We use this during testing so that we can access test data without
   // touching the disk.  (Other minor behaviors may also change in a test
   // environment.)
   extern bool InTestEnvironment;
   extern std::map<std::string, std::string>
      TestDataMap; // maps file name to contents
   extern std::map<std::string, unsigned int> URLFetchCount;
   extern bool (*FetchAsTestData)(const std::string& url, std::string* data);

   // This is a convenience function for fetching with a CurlNetFetcher.  It is
   // completely self-contained, so it may be called from multiple threads.
   extern bool FetchWithCurl(const std::string& url, std::string* data);

   // This is a convenience function for parsing a date time string from KML.
   // If there is an offset from UTC specified it will be returned so that
   // the caller can perform the calculations to the necessary time zone.  Note
   // that we do not run the tm struct through mktime after parsing, so some of
   // the values such as the day of the week may not be correct.
   extern bool ParseKMLDateTime(
      const std::string& date_time, struct tm* tm_val,
      int* offset_hours, int* offset_minutes, bool* negative_offset,
      std::string* error);
} // namespace kmldrawing

//******************************************************************************
//
// BEGIN CODE COPYRIGHTED BY GOOGLE
// This code has been modified from its original version.
// (see FalconView SVN history)
//
//******************************************************************************

// Copyright 2008, Google Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other materials provided with the distribution.
//  3. Neither the name of Google Inc. nor the names of its contributors may be
//     used to endorse or promote products derived from this software without
//     specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
// EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
// OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
// WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

bool CurlToString(const char* url, std::string* data);

// This class matches the NetFetcher concept used with kmlbase::NetCache.
class CurlNetFetcher : public kmlbase::NetFetcher {
 public:
  bool FetchUrl(const std::string& url, std::string* data) const {
    return CurlToString(url.c_str(), data);
  }
};

//******************************************************************************
//
// END CODE COPYRIGHTED BY GOOGLE
//
//******************************************************************************

#endif // #ifndef KML_DRAWING_LIBRARY_H_
