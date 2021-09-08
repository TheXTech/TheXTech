package ru.wohlsoft.thextech;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.VibrationEffect;
import android.os.Vibrator;

import java.io.File;

import androidx.appcompat.app.ActionBar;
import androidx.appcompat.app.AppCompatActivity;
import androidx.preference.Preference;
import androidx.preference.PreferenceFragmentCompat;
import androidx.preference.PreferenceManager;

public class GameSettings extends AppCompatActivity
{
    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.settings_activity);
        if (savedInstanceState == null)
        {
            getSupportFragmentManager()
                    .beginTransaction()
                    .replace(R.id.settings, new SettingsFragment())
                    .commit();
        }

        ActionBar actionBar = getSupportActionBar();
        if (actionBar != null)
            actionBar.setDisplayHomeAsUpEnabled(true);
    }

    public static class SettingsFragment extends PreferenceFragmentCompat
    {
        @Override
        public void onCreatePreferences(Bundle savedInstanceState, String rootKey)
        {
            setPreferencesFromResource(R.xml.root_preferences, rootKey);

            Preference button = getPreferenceManager().findPreference("setup_assets_path");
            if (button != null)
            {
                button.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener()
                {
                    @Override
                    public boolean onPreferenceClick(Preference arg0)
                    {
                        Context ctx = getContext();
                        if(ctx == null)
                            return false;// Null context, not allowed!
                        return selectAssetsPath(ctx, null);
                    }
                });
            }

            Preference fb_test = getPreferenceManager().findPreference("touchscreen_vibration_test");
            if (fb_test != null)
            {
                fb_test.setOnPreferenceClickListener(new Preference.OnPreferenceClickListener()
                {
                    @Override
                    public boolean onPreferenceClick(Preference arg0)
                    {
                        Context ctx = getContext();
                        if(ctx == null)
                            return false;// Null context, not allowed!

                        SharedPreferences setup = PreferenceManager.getDefaultSharedPreferences(ctx);
                        float vibratorStrength = Float.parseFloat(setup.getString("touchscreen_vibration_strength", "1.0"));
                        int vibratorLength = Integer.parseInt(setup.getString("touchscreen_vibration_length", "12"));

                        int vibeValue = Math.round(vibratorStrength * 255);

                        Vibrator v = (Vibrator)ctx.getSystemService(Context.VIBRATOR_SERVICE);

                        if (vibeValue > 255) {
                            vibeValue = 255;
                        }
                        if (vibeValue < 1) {
                            return false;
                        }
                        // Vibrate for X milliseconds
                        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O)
                            v.vibrate(VibrationEffect.createOneShot(vibratorLength, vibeValue));
                        else
                            v.vibrate(vibratorLength); //deprecated in API 26

                        return true;
                    }
                });
            }
        }
    }

    private static final String m_defaultAssetsDir = Environment.getExternalStorageDirectory().getAbsolutePath() + "/PGE Project/thextech";

    private static boolean isDirectoryExist(String dir)
    {
        File file = new File(dir);
        return file.exists() && file.isDirectory();
    }

    public static boolean isFileExist(String dir)
    {
        File file = new File(dir);
        return file.exists() && file.isFile();
    }

    public static boolean verifyAssetsPath(String path)
    {
        if(path.isEmpty())
            path = GameSettings.m_defaultAssetsDir;

        if(!isDirectoryExist(path))
            return false;

        if(!isDirectoryExist(path + "/graphics"))
            return false;

        if(!isDirectoryExist(path + "/sound"))
            return false;

        if(!isDirectoryExist(path + "/music"))
            return false;

        if(!isFileExist(path + "/intro.lvlx") && !isFileExist(path + "/intro.lvl"))
            return false;

        return true;
    }

    public static boolean selectAssetsPath(Context ctx, final Launcher l)
    {
        if(ctx == null)
            return false;// Null context, not allowed!

        SharedPreferences setup = PreferenceManager.getDefaultSharedPreferences(ctx);
        String gameAssetsPath = setup.getString("setup_assets_path", "");
        if(gameAssetsPath.isEmpty())
            gameAssetsPath = GameSettings.m_defaultAssetsDir;

        File file = new File(gameAssetsPath);
        if(!file.exists() || !file.isDirectory())
        {
            gameAssetsPath = Environment.getExternalStorageDirectory().getAbsolutePath();
        }

        OpenFileDialog fileDialog = new OpenFileDialog(ctx)
                .setDirectoryMode()
                .setFilter("gameinfo\\.ini")
                .setCurrentDirectory(gameAssetsPath)
                .setOpenDialogListener(new OpenFileDialog.OpenDialogListener()
                {
                    @Override
                    public void OnSelectedDirectory(Context ctx, String lastPath)
                    {
                        SharedPreferences setup = PreferenceManager.getDefaultSharedPreferences(ctx);
                        setup.edit().putString("setup_assets_path", lastPath).apply();
                        if(l != null)
                            l.updateOverlook();
                    }
                    @Override
                    public void OnSelectedFile(Context ctx, String fileName, String lastPath){}
                });
        fileDialog.show();
        return true;
    }
}
