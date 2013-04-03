package org.falconview.kmldrawer;

import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.view.View.OnTouchListener;
import android.view.Window;
import android.view.WindowManager;
import android.app.Activity;
import android.os.Bundle;
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
	    private static final String TAG = "DrawView";

	    List<Point> points = new ArrayList<Point>();
	    Paint paint = new Paint();

	    public DrawView(Context context) {
	        super(context);
	        setFocusable(true);
	        setFocusableInTouchMode(true);

	        //this.setOnTouchListener(this);

	        paint.setColor(Color.WHITE);
	        paint.setAntiAlias(true);
	    }

	    @Override
	    public void onDraw(Canvas canvas) {
	    	// paint background
	    	canvas.drawARGB(255, 0, 0, 0);
	    	
	    	// clear objects
	    	points.clear();
	    	
	        // draw KML (adds KML objects to draw)
	        
	        int height = canvas.getHeight();
	        int width = canvas.getWidth();
	        
	        drawKML(height, width);
	        
	        // draw background
	        
	    	// draw points
	        for (Point point : points) {
	            canvas.drawCircle(point.x, point.y, 5, paint);
	            // Log.d(TAG, "Painting: "+point);
	        }
	    }

	    public boolean onTouch(View view, MotionEvent event) {
	        // if(event.getAction() != MotionEvent.ACTION_DOWN)
	        // return super.onTouchEvent(event);
	        //Log.d(TAG, "point: " + point);
	    	addEllipse((int)event.getX(), (int)event.getY());
	        //nativeLog("new touch point: " + point);
	        invalidate();
	        return true;
	    }
	    
	    public void addEllipse(float x, float y) {
	        Point point = new Point();
	        point.x = x;
	        point.y = y;
	        points.add(point);
	        nativeLog("new KML point: " + point);
	    }
	}

	class Point {
	    float x, y;

	    @Override
	    public String toString() {
	        return x + ", " + y;
	    }
	}
	
    DrawView drawView;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        // initialize the drawer
        initDrawer("http://www.example.com/");
        
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
}