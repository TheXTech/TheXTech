package ru.wohlsoft.thextech;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.os.Environment;
import android.text.InputType;
import android.util.DisplayMetrics;
import android.widget.EditText;

import java.io.File;
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
//            "hidapi",
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

        if(setup.getBoolean("setup_show_controller_state", false))
            args.add("--show-controls");

        int showBatteryStatus = Integer.parseInt(setup.getString("setup_show_battery_status", "0"));
        if(showBatteryStatus > 0)
        {
            args.add("--show-battery-status");
            args.add(String.valueOf(showBatteryStatus));
        }

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

        setVibrationEnabled(setup.getBoolean("touchscreen_feedback_enabled", false));
        setVibrationStrength(Float.parseFloat(setup.getString("touchscreen_vibration_strength", "1.0")));
        setVibrationLength(Integer.parseInt(setup.getString("touchscreen_vibration_length", "12")));

        String gameAssetsPath = setup.getString("setup_assets_path", "");
        if(!gameAssetsPath.isEmpty())
        {
            File f = new File(gameAssetsPath);
            if (f.exists() && f.isDirectory()) {
                setGameAssetsPath(gameAssetsPath);
            }
        }

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
        setSdCardPath(Environment.getExternalStorageDirectory().getAbsolutePath());
        setAppDataPath(getApplication().getApplicationContext().getFilesDir().getAbsolutePath());

        DisplayMetrics displayMetrics = new DisplayMetrics();
        getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
        setScreenSize(SDLActivity.getDiagonal(), displayMetrics.widthPixels, displayMetrics.heightPixels);
    }

    @Override
    protected void onDestroy()
    {
        super.onDestroy();
        // Completely close the application.
        System.exit(0);
    }

    private void requestCheatShow()
    {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(R.string.cheat_dialog_title);

        // Set up the input
        final EditText input = new EditText(this);

        input.setInputType(InputType.TYPE_TEXT_FLAG_NO_SUGGESTIONS);
        builder.setView(input);

        builder.setPositiveButton(R.string.cheat_dialog_ok, new DialogInterface.OnClickListener()
        {
            @Override
            public void onClick(DialogInterface dialog, int which)
            {
                cheats_setBuffer(input.getText().toString());
                messageboxSelection[0] = 1;
                dialog.dismiss();
            }
        });

        builder.setNegativeButton(R.string.cheat_dialog_cancel, new DialogInterface.OnClickListener()
        {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                messageboxSelection[0] = 1;
                dialog.cancel();
            }
        });

        final AlertDialog dialog = builder.create();
        dialog.setCancelable(false);
        dialog.setOnDismissListener(new DialogInterface.OnDismissListener() {
            @Override
            public void onDismiss(DialogInterface unused) {
                synchronized (messageboxSelection) {
                    messageboxSelection.notify();
                }
            }
        });

        dialog.show();
    }

    public void requestCheat()
    {
        messageboxSelection[0] = -1;

        runOnUiThread(new Runnable()
        {
            @Override
            public void run() {
                requestCheatShow();
            }
        });

        synchronized (messageboxSelection) {
            try {
                messageboxSelection.wait();
            } catch (InterruptedException ex) {
                ex.printStackTrace();
            }
        }
    }

    public static native void setHardwareKeyboardPresence(int keyboard);
    public static native void setTouchScreenMode(int mode);
    public static native void setTouchScreenShowOnStart(boolean showOnStart);
    public static native void setScreenSize(double screenSize, double width, double height);
    public static native void setTouchPadStyle(int style);
    public static native void setSdCardPath(String path);
    public static native void setAppDataPath(String path);
    public static native void setGameAssetsPath(String path);
    // Send the cheat buffer line
    public static native void cheats_setBuffer(String line);
    // Touch-screen controller feeback
    public static native void setVibrationEnabled(boolean enabled);
    public static native void setVibrationStrength(float strength);
    public static native void setVibrationLength(int length);
}
