// Copyright (c) 1994-2013 Georgia Tech Research Corporation, Atlanta, GA
// This file is part of FalconView(R).

// FalconView(R) is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// FalconView(R) is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY {} without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public License
// along with FalconView(R).  If not, see <http://www.gnu.org/licenses/>.

// FalconView(R) is a registered trademark of Georgia Tech Research Corporation.

#include "kml_tests.h"

void TestShim::BeginDraw() {}
void TestShim::EndDraw() {}
void TestShim::BeginDrawElement(const kmldom::ElementPtr& element) {}
void TestShim::EndDrawElement(const kmldom::ElementPtr& element) {}
void TestShim::GetSurfaceSize(int* width, int* height) {}
void TestShim::GetMapBounds(double* bottom_lat, double* left_lon,
      double* top_lat, double* right_lon) {}
void TestShim::GetDegreesPerPixel(double* dpp_y, double* dpp_x) {}
void TestShim::GeoToSurface(double lat, double lon, int* x, int* y) {}
void TestShim::SurfaceToGeo(int x, int y, double* lat, double* lon) {}
void TestShim::CreatePen(
      const kmlbase::Color32& color, float width, kmldrawing::Pen** pen) {}
void TestShim::CreateBrush(
      const kmlbase::Color32& color, kmldrawing::Brush** brush) {}

void TestShim::CreateImageFromRawBytes(
      unsigned char* bytes, size_t length, kmldrawing::Image** image,
      bool will_be_used_for_geo /*= false*/)
{
   *image = &m_image;
}

void TestShim::DrawImage(const kmldrawing::Image& image,
      float left, float top, float right, float bottom, float rotation)
{
   ++m_draw_image_calls;
}

void TestShim::DrawImageGeo(const kmldrawing::Image& image,
   double west, double north, double east, double south)
{
}

void TestShim::DrawEllipse(float x, float y, float x_axis, float y_axis,
      const kmldrawing::Pen& pen, const kmldrawing::Brush& brush) {}
void TestShim::DrawPolyPolygon(
      const std::vector<std::pair<float, float>>& points,
      const std::vector<int>& point_counts,
      const kmldrawing::Pen& pen, const kmldrawing::Brush& brush) {}
void TestShim::DrawLines(const std::vector<std::pair<float, float>>& points,
      const kmldrawing::Pen& pen) {}
void TestShim::DrawString(const std::string& text, float x, float y,
   float size, const kmldrawing::Pen& pen) {}
void TestShim::ShowInformation(
      const std::string& title, const std::string& text) {}

void TestShim::SetTimer(int id, unsigned long ms)
{
   ++m_set_timer_calls;
}

void TestShim::ClearAllTimers() {}
void TestShim::Invalidate(void* ptr) {}

bool TestShim::Fetch(const std::string& uri, std::string* data)
{
   return false; //kmldrawing::FetchWithCurl(uri, data);
}

bool TestShim::ShouldRender(const kmldom::TimePrimitivePtr& time_primitive)
{
   return true;
}

void TestShim::LogMessage(const std::string& message)
{
}
