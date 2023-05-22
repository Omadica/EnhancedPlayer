package thor.dynacolor.testfisheye;

import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageView;

import com.fisheye.FisheyeImage;

public class MainActivity extends AppCompatActivity implements View.OnTouchListener {

    private ImageView org = null;
    private ImageView dewarp = null;
    private ImageView dewarp2 = null;
    private FisheyeImage dewarper = null;
    private FisheyeImage dewarper2 = null;

    private byte[] orgBitmap;
    private byte[] ptzBitmap;
    private byte[] n360Bitmap;

    private Bitmap ptzImage = null;
    private Bitmap n360Image = null;

    int fishImageWidth;
    int fishImageHeight;

    int n360ImageWidth;
    int n360ImageHeight;


    @Override
    protected void onDestroy() {
        super.onDestroy();
        dewarper.release();
        dewarper2.release();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        org = (ImageView) findViewById(R.id.imageView);
        org.setOnTouchListener(this);
        dewarp = (ImageView) findViewById(R.id.imageView2);
        dewarp2 = (ImageView) findViewById(R.id.imageView3);
        dewarp2.setOnTouchListener(this);

        fishImageWidth = 300;
        fishImageHeight = 300;
        Bitmap fishBitmap = decodeSampledBitmapFromResource(getResources(), R.drawable.fisheye, fishImageWidth, fishImageHeight);
        fishImageWidth = fishBitmap.getWidth();
        fishImageHeight = fishBitmap.getHeight();

        Log.d("FISH", "fish:" + fishImageWidth + "," + fishImageHeight);

        org.setImageBitmap(fishBitmap);

        orgBitmap = new byte[fishImageWidth * fishImageHeight * 3];
        for (int x = 0; x < fishImageWidth; x++) {
            for (int y = 0; y < fishImageHeight; y++) {
                int pixel = fishBitmap.getPixel(x, y);
                int redValue = 256 - Color.red(pixel);
                int blueValue = 256 - Color.blue(pixel);
                int greenValue = 256 - Color.green(pixel);

                int target = (y * fishImageWidth + x) * 3;
                orgBitmap[target] = (byte) redValue;
                orgBitmap[target + 1] = (byte) greenValue;
                orgBitmap[target + 2] = (byte) blueValue;
            }
        }

        dewarper = new FisheyeImage(fishImageWidth, fishImageHeight, 0, 24);
        dewarper2 = new FisheyeImage(fishImageWidth, fishImageHeight, 0, 24);
        int w = 2592;
        int h = 1944;
        int cx = 1287;
        int cy = 984;
        int d = 2010;
        float fl = 477;


        cx = (int) ((float) cx * fishImageWidth / w);
        cy = (int) ((float) cy * fishImageHeight / h);
        d = (int) ((float) d * fishImageWidth / w);
        fl = (int) (fl * fishImageWidth / w);
        w = fishImageWidth;
        h = fishImageHeight;

        dewarper.setParameters(w, h, cx, cy, d, fl, 1, 0);
        dewarper2.setParameters(w, h, cx, cy, d, fl, 1, 0);

//        n360ImageHeight = 150;
//        n360ImageWidth = dewarper.getOutImgW_360standard(360, 5, 95, n360ImageHeight);

        n360ImageWidth = 600;
        n360ImageHeight = dewarper2.getOutImgH_360standard(360, 5, 95, n360ImageWidth);

        Log.d("FISH", "360:" + n360ImageWidth + "," + n360ImageHeight);

        n360Bitmap = new byte[n360ImageWidth * n360ImageHeight * 3];
        n360Image = Bitmap.createBitmap(n360ImageWidth, n360ImageHeight, Bitmap.Config.ARGB_8888);

        ptzBitmap = new byte[fishImageWidth * fishImageHeight * 3];
        ptzImage = Bitmap.createBitmap(fishImageWidth, fishImageHeight, Bitmap.Config.ARGB_8888);

        dewarp.setImageBitmap(ptzImage);
        dewarp2.setImageBitmap(n360Image);

        getPTZImage(0, 0);
        get360Image();
    }

    private void get360Image() {
        dewarper2.standard360(0, 360, 1, 5, 95,
                orgBitmap, n360Bitmap,
                n360ImageWidth, n360ImageHeight);

        for (int y = 0; y < n360ImageHeight; y++) {
            for (int x = 0; x < n360ImageWidth; x++) {
                int target = (y * n360ImageWidth + x) * 3;
                int redValue = 256 - n360Bitmap[target];
                int greenValue = 256 - n360Bitmap[target + 1];
                int blueValue = 256 - n360Bitmap[target + 2];


                n360Image.setPixel(x, y, Color.argb(255, redValue, greenValue, blueValue));
            }
        }
    }

    public void getPTZImage(int pan, int tilt) {
        dewarper.dewarp(pan, tilt, 1.0f,
                orgBitmap, ptzBitmap,
                fishImageWidth, fishImageHeight);

        for(int y = 0; y < fishImageHeight; y++) {
            for (int x = 0; x < fishImageWidth; x++) {
                int target = (y * fishImageWidth + x) * 3;
                int redValue = 256 - ptzBitmap[target];
                int blueValue = 256 - ptzBitmap[target + 2];
                int greenValue = 256 - ptzBitmap[target + 1];

                ptzImage.setPixel(x, y, Color.argb(255, redValue, greenValue, blueValue));
            }
        }

        dewarp.invalidate();
    }

    private static int calculateInSampleSize(BitmapFactory.Options options,
                                             int reqWidth, int reqHeight) {
        final int height = options.outHeight;
        final int width = options.outWidth;
        int inSampleSize = 1;
        if (height > reqHeight || width > reqWidth) {
            final int heightRatio = Math.round((float) height / (float) reqHeight);
            final int widthRatio = Math.round((float) width / (float) reqWidth);
            inSampleSize = heightRatio < widthRatio ? heightRatio : widthRatio;
        }
        return inSampleSize;
    }

    private static Bitmap decodeSampledBitmapFromResource(Resources res, int resId,
                                                          int reqWidth, int reqHeight) {
        final BitmapFactory.Options options = new BitmapFactory.Options();
        options.inJustDecodeBounds = true;
        BitmapFactory.decodeResource(res, resId, options);
        options.inSampleSize = calculateInSampleSize(options, reqWidth, reqHeight);
        options.inJustDecodeBounds = false;

        reqWidth = options.outWidth / options.inSampleSize;
        reqHeight = options.outHeight / options.inSampleSize;

        return Bitmap.createScaledBitmap(BitmapFactory.decodeResource(res, resId, options), reqWidth, reqHeight, false);
    }

    @Override
    public boolean onTouch(View v, MotionEvent e) {
        float x = e.getX();
        float y = e.getY();
        int w = v.getWidth();
        int h = v.getHeight();

        int xi = (int) (x * fishImageWidth / w);
        int yi = (int) (y * fishImageHeight / h);

        if (xi >= 0 && xi < fishImageWidth && yi >= 0 && yi < fishImageHeight) {
            switch (e.getAction()) {
                case MotionEvent.ACTION_MOVE:
//                    case MotionEvent.ACTION_DOWN:
                    dewarper.getPTFromOutImg(xi, yi);
                    Log.d("POS", xi + "," + yi + "," + dewarper.getPanPt() + "," + dewarper.getTiltPt());
                    getPTZImage(dewarper.getPanPt(), dewarper.getTiltPt());
                    break;
            }
        }


        return true;
    }
}
