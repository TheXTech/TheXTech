package ru.wohlsoft.thextech;

import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.graphics.Point;
import android.os.Bundle;
import android.os.Environment;
import android.util.DisplayMetrics;
import android.view.WindowManager;

import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

import org.libsdl.app.SDLActivity;

import androidx.preference.PreferenceManager;

enum ControllerKeys
{
    key_BEGIN(0),
    key_start(0),
    key_left(1),
    key_right(2),
    key_up(3),
    key_down(4),
    key_run(5),
    key_jump(6),
    key_altrun(7),
    key_altjump(8),
    key_drop(9),
    key_END(10);

    private final int value;

    ControllerKeys(final int newValue)
    {
        value = newValue;
    }

    public int getValue()
    {
        return value;
    }
}

public class thextechActivity extends SDLActivity
{
    static boolean gameRunning = false;

    protected String[] getLibraries()
    {
        return new String[] {
            "hidapi",
//            "SDL2",
            "thextech"
        };
    }

    private String detectLanguage()
    {
        String lang = Locale.getDefault().toString();
        String[] langD = lang.split("_");
        if(langD.length >= 1)
            return langD[0];
        return "";
    }

    protected String[] getArguments()
    {
        List<String> args = new ArrayList<>();
        args.add("thextech"); // fake %0

        // Detect current language of the system
        String lang = detectLanguage();
        // TODO: NOT IMPLEMENTED YET
//        if(lang.length() >= 1)
//            args.add("--lang=" + lang);
        SharedPreferences setup = PreferenceManager.getDefaultSharedPreferences(getBaseContext());
        if(setup.getBoolean("enable_frame_skip", false))
            args.add("--frameskip");
        if(setup.getBoolean("disable_sound", false))
            args.add("--no-sound");
        if(setup.getBoolean("show_fps", false))
            args.add("--show-fps");
        if(setup.getBoolean("enable_max_fps", false))
            args.add("--max-fps");

        int speedRunMode = Integer.parseInt(setup.getString("setup_speedRunMode", "0"));
        if(speedRunMode > 0)
        {
            args.add("--speed-run-mode");
            args.add(String.valueOf(speedRunMode));

            if(setup.getBoolean("setup_sr_showStopwatchTransparent", false))
                args.add("--speed-run-semitransparent");
        }

        setTouchScreenMode(Integer.parseInt(setup.getString("setup_touchscreen_mode", "1")));
        setTouchScreenShowOnStart(setup.getBoolean("touchscreen_gamepad_showalways", false));
        setTouchPadStyle(Integer.parseInt(setup.getString("setup_touchscreen_style", "0")));

        String[] argsOut = new String[args.size()];
        args.toArray(argsOut);

        return argsOut;
    }

    @Override
    protected void onStart()
    {
        super.onStart();
        gameRunning = true;
        setHardwareKeyboardPresence(getResources().getConfiguration().keyboard);
    }


    @Override
    public void onStop()
    {
        super.onStop();
        gameRunning = false;
    }

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        this.setSdCardPath(Environment.getExternalStorageDirectory().getAbsolutePath());

        DisplayMetrics displayMetrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
        this.setScreenSize(SDLActivity.getDiagonal(), displayMetrics.widthPixels, displayMetrics.heightPixels);
    }

    @Override
    protected void onDestroy()
    {
        super.onDestroy();
        // Completely close the application.
        System.exit(0);
    }

    public static native void setHardwareKeyboardPresence(int keyboard);
    public static native void setTouchScreenMode(int mode);
    public static native void setTouchScreenShowOnStart(boolean showOnStart);
    public static native void setScreenSize(double screenSize, double width, double height);
    public static native void setTouchPadStyle(int style);
    public static native void setSdCardPath(String path);
}
