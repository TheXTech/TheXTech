package ru.wohlsoft.thextech;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
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
    private String levelToRun;
    private boolean editRequested = false;

    protected String[] getLibraries()
    {
        return new String[] {
//            "c++_shared",
//            "hidapi",
//            "SDL2",
            "thextech"
        };
    }

    private void detectLanguage()
    {
        String lang = Locale.getDefault().toString();
        String[] langD = lang.split("_");
        if(langD.length >= 2)
            setLanguageCodes(langD[0], langD[1]);
        else if(langD.length == 1)
            setLanguageCodes(langD[0], "");
    }

    protected String[] getArguments()
    {
        List<String> args = new ArrayList<>();
        args.add("thextech"); // fake %0

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

        String renderer = setup.getString("setup_renderer", "");
        if(!renderer.isEmpty())
        {
            args.add("--render");
            args.add(renderer);
        }

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

        if(editRequested)
            args.add("-e");

        if(!levelToRun.isEmpty())
            args.add(levelToRun);

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
        levelToRun = "";
        // Detect current language of the system
        detectLanguage();
        Intent intent = getIntent();
        Bundle extras = intent.getExtras();
        if(extras != null)
        {
            if(extras.containsKey("do-open-file"))
                levelToRun = extras.getString("do-open-file");

            if(extras.containsKey("edit-requested"))
                editRequested = true;
        }
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

    private void requestTextShow()
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
                textentry_setBuffer(input.getText().toString());
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

    public void requestText()
    {
        messageboxSelection[0] = -1;

        runOnUiThread(new Runnable()
        {
            @Override
            public void run() {
                requestTextShow();
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

    public static native void setScreenSize(double screenSize, double width, double height);
    public static native void setSdCardPath(String path);
    public static native void setAppDataPath(String path);
    public static native void setGameAssetsPath(String path);
    // Send the cheat buffer line
    public static native void textentry_setBuffer(String line);
    // set language settings
    public static native void setLanguageCodes(String lang, String country);
}
