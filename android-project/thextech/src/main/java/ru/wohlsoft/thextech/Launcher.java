package ru.wohlsoft.thextech;

import android.Manifest;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Shader;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Build;
import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import android.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.preference.PreferenceManager;

import android.os.Bundle;
import android.os.Environment;
import android.provider.Settings;
import android.util.Log;
import android.util.Pair;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.PopupMenu;
import android.widget.RelativeLayout;
import android.widget.Toast;

import com.google.android.material.floatingactionbutton.FloatingActionButton;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;
import java.util.Objects;

import javautil.FileUtils;


public class Launcher extends AppCompatActivity
{
    final String LOG_TAG = "TheXTech";
    public static final int READWRITE_PERMISSION_FOR_GAME = 1;
    public static final int READWRITE_PERMISSION_FOR_GAME_BY_INTENT = 2;
    public static final int READWRITE_PERMISSION_FOR_ADD_DIRECTORY = 3;
    private Context m_context = null;
    private String filePathToOpen;
    private String filePathToEdit;
    private boolean editRequested = false;

    /* ============ Animated background code ============ */
    private int m_bgAnimatorFrames = 1;
    private int m_bgAnimatorCurrentFrame = 0;
    private Bitmap m_bgAnimatorBitmap;
    private final UIUpdater m_bgAnimator = new UIUpdater(new Runnable()
    {
        @Override
        public void run()
        {
            m_bgAnimatorCurrentFrame++;
            if(m_bgAnimatorCurrentFrame >= m_bgAnimatorFrames)
                m_bgAnimatorCurrentFrame = 0;
            RelativeLayout launcher = findViewById(R.id.LauncherLayout);
            int fHeight = m_bgAnimatorBitmap.getHeight() / m_bgAnimatorFrames;
            int fOffset = m_bgAnimatorCurrentFrame * fHeight;
            Bitmap bitmap = Bitmap.createBitmap(m_bgAnimatorBitmap, 0, fOffset, m_bgAnimatorBitmap.getWidth(), fHeight);
            BitmapDrawable drawable = new BitmapDrawable(Launcher.this.getResources(), bitmap);
            drawable.setTileModeX(Shader.TileMode.REPEAT);
            drawable.getPaint().setFilterBitmap(false);
            launcher.setBackground(drawable);
        }
    });
    /* ================================================== */

    private static final int MENU_ADD = Menu.FIRST;
    private List<Pair<String, String>> menu_items = new ArrayList<Pair<String, String>>();;

    @Override
    protected void onCreate(Bundle savedInstanceState)
    {
        if(!isTaskRoot())
        {
            finish();
            return;
        }
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_launcher);
        initUiSetup();
        filePathToOpen = "";
        filePathToEdit = "";
        handleFileIntent();
    }

    @Override
    protected void onResume()
    {
        super.onResume();
        updateOverlook();
    }

    private void initUiSetup()
    {
        Button startGame = findViewById(R.id.startGame);
        startGame.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View view)
            {
                OnStartGameClick(view);
            }
        });

        Button gameSettings = findViewById(R.id.gameSettings);
        gameSettings.setOnClickListener(
        new View.OnClickListener()
        {
            @Override
            public void onClick(View view)
            {
                Intent myIntent = new Intent(Launcher.this, GameSettings.class);
                Launcher.this.startActivity(myIntent);
            }
        });

        updateOverlook();
        reloadAssetsList();

        FloatingActionButton fab = findViewById(R.id.selectGame);
        fab.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View view)
            {
                int i = 0;
                PopupMenu menu = new PopupMenu(Launcher.this, view);

                MenuItem dirSelect = menu.getMenu().add(0, MENU_ADD, Menu.NONE, R.string.launcher_gameSelect_pickTheDir);
                dirSelect.setIcon(R.drawable.add_directory);

                for(i = 0; i < menu_items.size(); i++)
                {
                    Pair<String, String> e = menu_items.get(i);
                    String gameName = e.first;
                    String gamePath = e.second;

                    Log.d(LOG_TAG, "Assets name: " + gameName + "; path: " + gamePath);

                    if(!GameSettings.isDirectoryExist(gamePath))
                        continue; // Skip invalid assets

                    MenuItem it = menu.getMenu().add(0, MENU_ADD + i + 1, Menu.NONE, gameName);

                    String iconPath = gamePath + "/graphics/ui/icon/thextech_128.png";
                    if(GameSettings.isFileExist(iconPath)) // Add icon only when it exists and accessible
                    {
                        Log.d(LOG_TAG, "Icon file exists: " + iconPath);
                        Drawable d = Drawable.createFromPath(iconPath);
                        it.setIcon(d);
                    }
                    else
                        Log.d(LOG_TAG, "Icon file DOES NOT EXISTS: " + iconPath);
                }

                int CLEAR_ITEM = MENU_ADD + i + 1;
                MenuItem clearItem = menu.getMenu().add(0, CLEAR_ITEM, Menu.NONE, R.string.launcher_gameSelect_clearList);
                clearItem.setIcon(R.drawable.clear_list);

                menu.setOnMenuItemClickListener(new PopupMenu.OnMenuItemClickListener()
                {
                    @Override
                    public boolean onMenuItemClick(MenuItem item)
                    {
                        int id = item.getItemId();

                        if(id == MENU_ADD)
                        {
                            if(checkFilePermissions(READWRITE_PERMISSION_FOR_ADD_DIRECTORY) || !hasManageAppFS())
                                return false;
                            GameSettings.selectAssetsPath(Launcher.this, Launcher.this);
                        }
                        else if(id == CLEAR_ITEM)
                        {
                            AlertDialog.Builder b = new AlertDialog.Builder(Launcher.this);
                            b.setTitle(R.string.launcher_gameselect_clearlist_title);
                            b.setMessage(R.string.launcher_gameselect_clearlist_question);
                            b.setNegativeButton(android.R.string.no, null);
                            b.setPositiveButton(android.R.string.yes, new DialogInterface.OnClickListener()
                            {
                                public void onClick(DialogInterface dialog, int whichButton)
                                {
                                    SharedPreferences setup = PreferenceManager.getDefaultSharedPreferences(getBaseContext());
                                    setup.edit().putString("setup_assets_list", "").apply();
                                    reloadAssetsList();
                                    Toast.makeText(getApplicationContext(), getString(R.string.launcher_gameselect_toast_listclean), Toast.LENGTH_SHORT).show();
                                }
                            });
                            b.show();
                        }
                        else
                        {
                            int itemId = id - 2;
                            String gameName = menu_items.get(itemId).first;
                            String gamePath = menu_items.get(itemId).second;

                            Toast.makeText(getApplicationContext(), String.format(getString(R.string.launcher_gameselect_toast_selected), gameName), Toast.LENGTH_SHORT).show();
                            SharedPreferences setup = PreferenceManager.getDefaultSharedPreferences(getBaseContext());
                            setup.edit().putString("setup_assets_path", gamePath).apply();
                            updateOverlook();
                        }

                        return true;
                    }
                });

                if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q)
                    menu.setForceShowIcon(true);

                menu.show();
            }
        });

    }

    private void handleFileIntent()
    {
        Intent intent = getIntent();
        String scheme = intent.getScheme();
        if(scheme != null)
        {
            intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            if(checkFilePermissions(READWRITE_PERMISSION_FOR_GAME_BY_INTENT) || !hasManageAppFS())
                return;

            if(m_context == null)
                m_context = getApplicationContext();

            FileUtils utils = new FileUtils(this.m_context);
            filePathToOpen = utils.getPath(intent.getData());

            editRequested = Objects.equals(intent.getAction(), Intent.ACTION_EDIT);

            Log.d(LOG_TAG, "Got a file: " + filePathToOpen + ";");

            if(utils.isInternalCopy()) // File was coped into internal storage, resources are unavailable!
            {
                DialogInterface.OnClickListener dialogClickListener = new DialogInterface.OnClickListener()
                {
                    @Override
                    public void onClick(DialogInterface dialog, int which)
                    {
                        switch (which){
                            case DialogInterface.BUTTON_POSITIVE:
                                tryStartGame(m_context);
                                break;
                            case DialogInterface.BUTTON_NEGATIVE:
                                break;
                        }
                    }
                };

                AlertDialog.Builder builder = new AlertDialog.Builder(this);
                builder.setMessage(R.string.launcher_openfile_indirectly)
                        .setPositiveButton(android.R.string.yes, dialogClickListener)
                        .setNegativeButton(android.R.string.no, dialogClickListener)
                        .show();
            }
            else // File is accessible directly, do open it normally
                tryStartGame(m_context);
        }
    }

    public void OnStartGameClick(View view)
    {
        if(m_context == null)
            m_context = view.getContext();

        // Here, thisActivity is the current activity
        if(checkFilePermissions(READWRITE_PERMISSION_FOR_GAME) || !hasManageAppFS())
            return;

        tryStartGame(m_context);
    }

    private void tryStartGame(Context context)
    {
        assert(context != null);

        SharedPreferences setup = PreferenceManager.getDefaultSharedPreferences(getBaseContext());
        String gameAssetsPath = setup.getString("setup_assets_path", "");

        if(!GameSettings.verifyAssetsPath(gameAssetsPath))
        {
            DialogInterface.OnClickListener dialogClickListener = new DialogInterface.OnClickListener()
            {
                @Override
                public void onClick(DialogInterface dialog, int which)
                {
                    switch (which){
                        case DialogInterface.BUTTON_POSITIVE:
                            GameSettings.selectAssetsPath(Launcher.this, Launcher.this);
                            break;
                        case DialogInterface.BUTTON_NEGATIVE:
                            //No button clicked
                            break;
                    }
                }
            };

            AlertDialog.Builder builder = new AlertDialog.Builder(context);
            builder.setMessage(R.string.launcher_no_resources_question)
                    .setPositiveButton(android.R.string.yes, dialogClickListener)
                    .setNegativeButton(android.R.string.no, dialogClickListener)
                    .show();
            return;
        }

        startGame();
    }

    public void reloadAssetsList()
    {
        SharedPreferences setup = PreferenceManager.getDefaultSharedPreferences(getBaseContext());
        String gameAssetsPath = setup.getString("setup_assets_list", "");

        menu_items.clear();

        if(!gameAssetsPath.isEmpty())
        {
            try
            {
                JSONObject jObject = new JSONObject(gameAssetsPath);
                JSONArray jArray = jObject.getJSONArray("assets");
                for(int i = 0; i < jArray.length(); i++)
                {
                    JSONObject oneObject = jArray.getJSONObject(i);
                    String gameName = oneObject.getString("game");
                    String gamePath = oneObject.getString("path");
                    menu_items.add(new Pair<String, String>(gameName, gamePath));
                }
            }
            catch (JSONException e)
            {
                e.printStackTrace();
            }
        }
    }

    public void updateOverlook()
    {
        SharedPreferences setup = PreferenceManager.getDefaultSharedPreferences(getBaseContext());
        String gameAssetsPath = setup.getString("setup_assets_path", "");

        String logoImagePath = "graphics/ui/MenuGFX2.png";
        String assetsIcon = "graphics/ui/icon/thextech_128.png";
        String bgImagePath = "graphics/background2/background2-2.png";
        int bgImageFrames = 1;
        int bgImageFramesDelay = 125;

        String giPath = gameAssetsPath + "/gameinfo.ini";
        boolean hasGameInfo = !gameAssetsPath.isEmpty() && GameSettings.isFileExist(giPath);
        if(hasGameInfo)
        {
            IniFile gi = new IniFile(giPath);
            logoImagePath = gi.getString("android", "logo", logoImagePath);
            bgImagePath = gi.getString("android", "background", bgImagePath);
            bgImageFrames = gi.getInt("android", "background-frames", 1);
            bgImageFramesDelay = gi.getInt("android", "background-delay", 125);
        }

        String bgPath = gameAssetsPath + "/" + bgImagePath;
        String logoPath = gameAssetsPath + "/" + logoImagePath;
        String iconPath = gameAssetsPath + "/" + assetsIcon;

        m_bgAnimator.stopUpdates();

        RelativeLayout launcher = findViewById(R.id.LauncherLayout);
        if(!gameAssetsPath.isEmpty() && GameSettings.isFileExist(bgPath))
        {
            Bitmap bitmap = BitmapFactory.decodeFile(bgPath);
            if(bgImageFrames > 1)
            {
                m_bgAnimatorBitmap = Bitmap.createBitmap(bitmap);
                bitmap = Bitmap.createBitmap(bitmap, 0, 0, bitmap.getWidth(), bitmap.getHeight() / bgImageFrames);
                m_bgAnimatorFrames = bgImageFrames;
                m_bgAnimatorCurrentFrame = 0;
                m_bgAnimator.setInterval(bgImageFramesDelay);
            }
            BitmapDrawable drawable = new BitmapDrawable(this.getResources(), bitmap);
            drawable.setTileModeX(Shader.TileMode.REPEAT);
            drawable.getPaint().setFilterBitmap(false);
            launcher.setBackground(drawable);
            if(bgImageFrames > 1)
                m_bgAnimator.startUpdates();
        }
        else
            launcher.setBackgroundResource(R.drawable.background);

        ImageView gameLogo = findViewById(R.id.gameLogo);
        if(!gameAssetsPath.isEmpty() && GameSettings.isFileExist(logoPath))
        {
            Bitmap bitmap = BitmapFactory.decodeFile(logoPath);
            BitmapDrawable drawable = new BitmapDrawable(this.getResources(), bitmap);
            drawable.getPaint().setFilterBitmap(false);
            gameLogo.setImageDrawable(drawable);
        }
        else
            gameLogo.setImageResource(R.drawable.logo);

        FloatingActionButton fab = findViewById(R.id.selectGame);
        if(!gameAssetsPath.isEmpty() && GameSettings.isFileExist(iconPath))
        {
            Drawable d = Drawable.createFromPath(iconPath);
            fab.setImageDrawable(d);
        }
        else
            fab.setImageResource(R.drawable.add_directory);
    }

    private boolean checkFilePermissions(int requestCode)
    {
        final int grant = PackageManager.PERMISSION_GRANTED;
        final String exStorage = Manifest.permission.WRITE_EXTERNAL_STORAGE;

        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU)
            return false; /* Has no effect, the manage file storage permission is used instead of this */

        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.M)
        {
            if(ContextCompat.checkSelfPermission(this, exStorage) == grant)
                Log.d(LOG_TAG, "File permission is granted");
            else
                Log.d(LOG_TAG, "File permission is revoked");
        }

//      // if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN)
//        {
        if(ContextCompat.checkSelfPermission(this, exStorage) == grant)
            return false;

        // Should we show an explanation?
        if(ActivityCompat.shouldShowRequestPermissionRationale(this, exStorage))
        {
            // Show an explanation to the user *asynchronously* -- don't block
            // this thread waiting for the user's response! After the user
            // sees the explanation, try again to request the permission.
            AlertDialog.Builder b = new AlertDialog.Builder(this);
            b.setTitle("Permission denied");
            b.setMessage("Sorry, but permission is denied!\n"+
                         "Please, check the External Storage access permission to the game!");
            b.setNegativeButton(android.R.string.ok, null);
            b.show();
            return true;
        }
        else
        {
            // No explanation needed, we can request the permission.
            ActivityCompat.requestPermissions(this, new String[] { exStorage }, requestCode);
            // MY_PERMISSIONS_REQUEST_READ_EXTERNAL_STORAGE
            // MY_PERMISSIONS_REQUEST_READ_EXTERNAL_STORAGE is an
            // app-defined int constant. The callback method gets the
            // result of the request.
        }

        return true;
//        } // if JELLY_BEAN

//        return false;
    }

    public boolean hasManageAppFS()
    {
        if(Build.VERSION.SDK_INT >= 30)
        {
            if(Environment.isExternalStorageManager())
                return true;

            AlertDialog.Builder b = new AlertDialog.Builder(this);
            b.setTitle(R.string.managePermExplainTitle);
            b.setMessage(R.string.managePermExplainText);
            b.setNegativeButton(android.R.string.ok, new DialogInterface.OnClickListener()
            {
                public void onClick(DialogInterface dialog, int whichButton)
                {
                    Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION);
                    String pName = getPackageName();
                    Uri uri = Uri.fromParts("package", pName, null);
                    intent.setData(uri);
                    intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    startActivity(intent);
                }
            });
            b.show();

            return false;
        }

        return true;
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults)
    {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);

        if(grantResults.length == 0)
            return;
        if(!permissions[0].equals(Manifest.permission.WRITE_EXTERNAL_STORAGE))
            return;
        if(grantResults[0] != PackageManager.PERMISSION_GRANTED)
            return;
        if(!hasManageAppFS())
            return;

        switch(requestCode)
        {
        case READWRITE_PERMISSION_FOR_GAME:
            tryStartGame(m_context);
            break;

        case READWRITE_PERMISSION_FOR_ADD_DIRECTORY:
            GameSettings.selectAssetsPath(Launcher.this, Launcher.this);
            break;
        }
    }

    public void startGame()
    {
        Intent myIntent = new Intent(Launcher.this, thextechActivity.class);
        if(!filePathToOpen.isEmpty())
            myIntent.putExtra("do-open-file", filePathToOpen);

        if(editRequested)
            myIntent.putExtra("edit-requested", true);

//        myIntent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
//        myIntent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        myIntent.addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP);
        Launcher.this.startActivity(myIntent);
        Launcher.this.finish();
    }
}
