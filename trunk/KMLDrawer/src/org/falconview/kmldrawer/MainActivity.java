package org.falconview.kmldrawer;

import java.io.ByteArrayOutputStream;
import java.io.InputStream;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Rect;
import android.graphics.RectF;
import android.net.Uri;
import android.os.Bundle;
import android.os.StrictMode;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.Window;
import android.view.WindowManager;

public class MainActivity extends Activity {
	private native void nativeLog(String logThis);
	private native void initDrawer(String kml_url);
	private native void drawKML(int height, int width);
	private native void setGeoBounds(float ul_lat, float ul_lon, float lr_lat, float lr_lon);
	private native float[] getGeoBounds();

	static {
		System.loadLibrary("KMLShim");
	}

	public class DrawView extends View implements OnTouchListener {
		//private static final String TAG = "DrawView";

		List<Point> points = new ArrayList<Point>();
		List<Line> lines = new ArrayList<Line>();
		List<Image> images = new ArrayList<Image>();

		Paint paint = new Paint();

		public DrawView(Context context) {
			super(context);
			setFocusable(true);
			setFocusableInTouchMode(true);

			this.setOnTouchListener(this);

			paint.setColor(Color.WHITE);
			paint.setAntiAlias(true);
		}
		
		int height, width;

		@Override
		public void onDraw(Canvas canvas) {
			// draw background
			canvas.drawARGB(255, 0, 0, 0);

			// clear objects
			points.clear();
			lines.clear();
			images.clear();

			// call drawKML in the native library
			height = canvas.getHeight();
			width = canvas.getWidth();
			drawKML(height, width);
			
			// draw images
			for (Image image : images) {
				// TODO: rotation
				// TODO: don't allocate these objects during the draw
				Bitmap bmp = cachedBitmaps.get(image.index);
				Rect src = new Rect(0, 0, bmp.getWidth(), bmp.getHeight());
				RectF dst = new RectF(image.left, image.top, image.right, image.bottom);
				canvas.drawBitmap(bmp, src, dst, paint);
			}
			
			// draw lines
			for (Line line : lines) {
				canvas.drawLine(line.a.x, line.a.y, line.b.x, line.b.y, paint);
			}

			// draw points
			for (Point point : points) {
				canvas.drawCircle(point.x, point.y, 5, paint);
				// Log.d(TAG, "Painting: "+point);
			}
		}
		
		private static final int INVALID_POINTER_ID = -1;
		private int pointerA = INVALID_POINTER_ID;
		private int pointerB = INVALID_POINTER_ID;
		private float pointerAStartX, pointerAStartY, pointerBStartX, pointerBStartY;
		private float pointerAEndX, pointerAEndY, pointerBEndX, pointerBEndY;

		public boolean onTouch(View view, MotionEvent event) {
			switch (event.getActionMasked()) {

			case MotionEvent.ACTION_DOWN: {
				// save pointer A starting state

				pointerAStartX = event.getX();
				pointerAStartY = event.getY();
				pointerA = event.getPointerId(0);
				pointerB = INVALID_POINTER_ID; // in case it wasn't reset before

				break;
			}
			case MotionEvent.ACTION_POINTER_DOWN: {
				// save pointer B starting state

				if (event.getPointerCount() < 2)
					break; // ???

				int indexA = event.findPointerIndex(pointerA);
				if (indexA == INVALID_POINTER_ID)
					break; // ???
				int indexB = indexA == 0 ? 1 : 0;

				pointerBStartX = event.getX(indexB);
				pointerBStartY = event.getY(indexB);
				pointerB = event.getPointerId(indexB);

				break;
			}
			case MotionEvent.ACTION_POINTER_UP: {
				if (pointerA != INVALID_POINTER_ID
						&& pointerB != INVALID_POINTER_ID) {
					// save pointer ending states

					int indexA = event.findPointerIndex(pointerA);
					pointerAEndX = event.getX(indexA);
					pointerAEndY = event.getY(indexA);
					
					int indexB = event.findPointerIndex(pointerB);
					pointerBEndX = event.getX(indexB);
					pointerBEndY = event.getY(indexB);

					// leave pointerB set so that we know that ACTION_UP ends a
					// zoom
				} else {
					// what happened???
					pointerA = INVALID_POINTER_ID;
					pointerB = INVALID_POINTER_ID;
				}

				break;
			}
			case MotionEvent.ACTION_UP: {
				if (pointerA != INVALID_POINTER_ID
						&& pointerB == INVALID_POINTER_ID) {
					// panning

					// calculate degrees per pixel
					float[] bounds = getGeoBounds();
					float dppX = (bounds[3] - bounds[1]) / width;
					float dppY = (bounds[0] - bounds[2]) / height;

					// calculate change in degrees
					float dx = dppX * (pointerAStartX - event.getX());
					float dy = dppY * (event.getY() - pointerAStartY);

					// reset bounds

					bounds[0] += dy;
					bounds[2] += dy;
					bounds[1] += dx;
					bounds[3] += dx;

					setGeoBounds(bounds[0], bounds[1], bounds[2], bounds[3]);

					invalidate();
				} else if (pointerA != INVALID_POINTER_ID
						&& pointerB != INVALID_POINTER_ID) {
					// zooming

					// calculate zoom factor

					float dxStart = pointerAStartX - pointerBStartX;
					float dyStart = pointerAStartY - pointerBStartY;
					float d2Start = dxStart * dxStart + dyStart * dyStart;

					float dxEnd = pointerAEndX - pointerBEndX;
					float dyEnd = pointerAEndY - pointerBEndY;
					float d2End = dxEnd * dxEnd + dyEnd * dyEnd;

					float zoom = (float) Math.sqrt(d2Start / d2End);
					
					// calculate screen center of zoom as a fraction from edge
					
					float centerX = (pointerAStartX + pointerBStartX)/2.0f;
					float centerY = (pointerAStartY + pointerBStartY)/2.0f;
					
					float xFrac = centerX/width;
					float yFrac = centerY/height;
					
					// set new bounds
					
					float[] bounds = getGeoBounds();

					float geoWidth = bounds[3] - bounds[1];
					float geoHeight = bounds[0] - bounds[2];
					
					float geoCenterX = bounds[1] + xFrac*geoWidth;
					float geoCenterY = bounds[0] - yFrac*geoHeight;
					
					bounds[0] = geoCenterY + zoom*geoHeight/2.0f;
					bounds[1] = geoCenterX - zoom*geoWidth/2.0f;
					bounds[2] = geoCenterY - zoom*geoHeight/2.0f;
					bounds[3] = geoCenterX + zoom*geoWidth/2.0f;
					
					setGeoBounds(bounds[0], bounds[1], bounds[2], bounds[3]);
					
					invalidate();
				}

				pointerA = INVALID_POINTER_ID;
				pointerB = INVALID_POINTER_ID;

				break;
			}
			case MotionEvent.ACTION_CANCEL: {
				pointerA = INVALID_POINTER_ID;
				pointerB = INVALID_POINTER_ID;
				break;
			}
			}

			return true;
		}

		public void addEllipse(float x, float y) {
			Point point = new Point();
			point.x = x;
			point.y = y;
			points.add(point);
		}

		public void addLine(float x1, float y1, float x2, float y2) {
			Line line = new Line();

			Point point1 = new Point();
			point1.x = x1;
			point1.y = y1;
			line.a = point1;

			Point point2 = new Point();
			point2.x = x2;
			point2.y = y2;
			line.b = point2;

			lines.add(line);
		}
		
		public void addImage(int index, float left, float top, float right, float bottom, float rotation) {
			Image image = new Image(index, left, top, right, bottom, rotation);
			images.add(image);
		}
	}

	class Point {
		float x, y;

		@Override
		public String toString() {
			return x + ", " + y;
		}
	}

	class Line {
		Point a, b;

		@Override
		public String toString() {
			return a.x + ", " + a.y + " - " + b.x + ", " + b.y;
		}
	}
	
	List<Bitmap> cachedBitmaps = new ArrayList<Bitmap>();
	
	class Image {
		float left, top, right, bottom, rotation;
		int index;
		
		public Image(int index, float left, float top, float right, float bottom, float rotation) {
			this.index = index;
			this.left = left;
			this.top = top;
			this.right = right;
			this.bottom = bottom;
			this.rotation = rotation;
		}
	}

	DrawView drawView;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		// allow networking on main thread for now (Ouch!) TODO: make this right!
		// for better way, see http://stackoverflow.com/questions/6343166/android-os-networkonmainthreadexception
		StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder().permitAll().build();
		StrictMode.setThreadPolicy(policy);

		// we currently only support fetching by URL
		String url = "http://osm-kml.appspot.com/static/osm.kml"; // default 
		Intent intent = getIntent();
		Uri data = intent.getData();
		if (data != null)
	    	url = data.toString();

		// initialize the drawer
	    initDrawer(url);

		// Set full screen view
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN);
		requestWindowFeature(Window.FEATURE_NO_TITLE);

		drawView = new DrawView(this);
		setContentView(drawView);
		drawView.requestFocus();
	}
	
	public void addEllipse(float x, float y) {
		drawView.addEllipse(x, y);
	}

	public void addLine(float x1, float y1, float x2, float y2) {
		drawView.addLine(x1, y1, x2, y2);
	}
	
	public void addImage(int index, float left, float top, float right, float bottom, float rotation) {
		drawView.addImage(index, left, top, right, bottom, rotation);
	}
	
	public byte[] fetchURL(String url_string) {
		// calling nativeLog here leads to an eventual crash
		
		ByteArrayOutputStream baos = new ByteArrayOutputStream();

		try {
			URL url = new URL(url_string);
			InputStream is = url.openStream();
			byte[] byteChunk = new byte[4096];
			int n;
			while ((n = is.read(byteChunk)) > 0) {
				baos.write(byteChunk, 0, n);
			}
		} catch (Exception ex) { // TODO
		}

		return baos.toByteArray();
	}
	
	public int createImage(byte[] bytes) {
		Bitmap bmp = BitmapFactory.decodeByteArray(bytes, 0, bytes.length);
		cachedBitmaps.add(bmp);
		return cachedBitmaps.size() - 1;
	}
}
