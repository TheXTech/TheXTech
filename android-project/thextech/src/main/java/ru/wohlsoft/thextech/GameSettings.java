package ru.wohlsoft.thextech;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.VibrationEffect;
import android.os.Vibrator;
import android.util.Pair;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;

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
        }
    }

    private static final String m_defaultAssetsDir = Environment.getExternalStorageDirectory().getAbsolutePath() + "/PGE Project/thextech";

    public static boolean isDirectoryExist(String dir)
    {
        File file = new File(dir);
        return file.exists() && file.isDirectory();
    }

    public static boolean isFileExist(String f)
    {
        File file = new File(f);
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

    public static JSONArray sortAssetsList(JSONArray inArr)
    {
        try
        {
            ArrayList<JSONObject> array = new ArrayList<JSONObject>();

            for(int i = 0; i < inArr.length(); i++)
                array.add(inArr.getJSONObject(i));

            Collections.sort(array, new Comparator<JSONObject>()
            {
                @Override
                public int compare(JSONObject lhs, JSONObject rhs)
                {
                    try
                    {
                        return (lhs.getString("game").toLowerCase().compareTo(rhs.getString("game").toLowerCase()));
                    }
                    catch (JSONException e)
                    {
                        e.printStackTrace();
                        return 0;
                    }
                }
            });

            JSONArray outArray = new JSONArray();

            for(int i = 0; i < array.size(); i++)
                outArray.put(array.get(i));

            return outArray;
        }
        catch (JSONException e)
        {
            e.printStackTrace();
            return inArr;
        }
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

                        // Update list of assets (if needed)
                        String gameName = "<untitled game>";
                        String giPath = lastPath + "/gameinfo.ini";

                        boolean hasGameInfo = !lastPath.isEmpty() && GameSettings.isFileExist(giPath);
                        if(hasGameInfo)
                        {
                            IniFile gi = new IniFile(giPath);
                            gameName = gi.getString("game", "title", gameName);
                        }

                        // Add assets if missing
                        String gameAssetsPath = setup.getString("setup_assets_list", "");
                        try
                        {
                            boolean updateNeeded = false;
                            if(gameAssetsPath.isEmpty()) // Create new list
                            {
                                JSONObject jObject = new JSONObject();
                                JSONArray jArray = new JSONArray();
                                JSONObject oneObject = new JSONObject();
                                oneObject.put("game", gameName);
                                oneObject.put("path", lastPath);
                                jArray.put(oneObject);
                                jObject.put("assets", jArray);
                                setup.edit().putString("setup_assets_list", jObject.toString()).apply();
                                updateNeeded = true;
                            }
                            else // Append if not exists
                            {
                                JSONObject jObject = new JSONObject(gameAssetsPath);
                                JSONArray jArray = jObject.getJSONArray("assets");
                                updateNeeded = true;

                                for(int i = 0; i < jArray.length(); i++)
                                {
                                    JSONObject oneObject = jArray.getJSONObject(i);
                                    String tPath = oneObject.getString("path");
                                    if(tPath.equals(lastPath))
                                    {
                                        updateNeeded = false;
                                        break;
                                    }
                                }

                                if(updateNeeded) // Append missing item
                                {
                                    JSONObject oneObject = new JSONObject();
                                    oneObject.put("game", gameName);
                                    oneObject.put("path", lastPath);
                                    jArray.put(oneObject);
                                    jObject.put("assets", sortAssetsList(jArray));
                                    setup.edit().putString("setup_assets_list", jObject.toString()).apply();
                                }
                            }

                            if(updateNeeded && l != null)
                                l.reloadAssetsList();
                        }
                        catch (JSONException e)
                        {
                            e.printStackTrace();
                        }
                    }
                    @Override
                    public void OnSelectedFile(Context ctx, String fileName, String lastPath){}
                });
        fileDialog.show();
        return true;
    }
}
