package org.falconview.kmldrawer;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.os.Bundle;
import android.os.StrictMode;
import android.os.StrictMode.ThreadPolicy;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.Window;
import android.view.WindowManager;

public class MainActivity extends Activity {
	private native void nativeLog(String logThis);

	private native void initDrawer(String kml_url);

	private native void drawKML(int height, int width);

	static {
		System.loadLibrary("KMLShim");
	}

	public class DrawView extends View implements OnTouchListener {
		//private static final String TAG = "DrawView";

		List<Point> points = new ArrayList<Point>();
		List<Line> lines = new ArrayList<Line>();
		Paint paint = new Paint();

		public DrawView(Context context) {
			super(context);
			setFocusable(true);
			setFocusableInTouchMode(true);

			// this.setOnTouchListener(this);

			paint.setColor(Color.WHITE);
			paint.setAntiAlias(true);
		}

		@Override
		public void onDraw(Canvas canvas) {
			// draw background
			canvas.drawARGB(255, 0, 0, 0);

			// clear objects
			points.clear();
			lines.clear();

			// draw KML (adds KML objects to draw)

			int height = canvas.getHeight();
			int width = canvas.getWidth();

			drawKML(height, width);

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

		public boolean onTouch(View view, MotionEvent event) {
			// if(event.getAction() != MotionEvent.ACTION_DOWN)
			// return super.onTouchEvent(event);
			// Log.d(TAG, "point: " + point);
			addEllipse((int) event.getX(), (int) event.getY());
			// nativeLog("new touch point: " + point);
			invalidate();
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

	DrawView drawView;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		// allow networking on main thread for now (Ouch!)
		// for better way, see http://stackoverflow.com/questions/6343166/android-os-networkonmainthreadexception
		StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder().permitAll().build();
		StrictMode.setThreadPolicy(policy);

		// initialize the drawer
		initDrawer("http://osm-kml.appspot.com/static/us_states.kml");

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
	
	public String fetchURL(String url_string) {
		// CALLING nativeLog here causes crashes down the line
		
		String inputLine, output = "";
		StringBuilder sbuilder = new StringBuilder();

		try {
			URL url = new URL(url_string);
			BufferedReader in = new BufferedReader(new InputStreamReader(
					url.openStream()));
			while ((inputLine = in.readLine()) != null) {
				sbuilder.append(inputLine);
			}
			in.close();
			output = sbuilder.toString();
		} catch (MalformedURLException ex) {
			output = "MalformedURLException";
		} catch (IOException ex) {
			output = "IOException";
		} catch (Exception ex) {
			output = "Exception: " + ex.toString();
		}

		return output;
	}
}
