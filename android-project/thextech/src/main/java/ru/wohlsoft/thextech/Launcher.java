package ru.wohlsoft.thextech;

import android.Manifest;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Shader;
import android.graphics.drawable.BitmapDrawable;
import android.net.Uri;
import android.os.Build;
import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import androidx.preference.PreferenceManager;

import android.os.Bundle;
import android.os.Environment;
import android.provider.Settings;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.RelativeLayout;

public class Launcher extends AppCompatActivity
{
    final String LOG_TAG = "TheXTech";
    public static final int READWRITE_PERMISSION_FOR_GAME = 1;
    public static final int READWRITE_PERMISSION_FOR_GAME_BY_INTENT = 2;
    private Context m_context = null;
    private String filePathToOpen;

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
            if(scheme.equals(ContentResolver.SCHEME_FILE))
            {
                Uri url = intent.getData();
                if(url != null)
                {
                    Log.d(LOG_TAG, "Got a file: " + url + ";");
                    filePathToOpen = url.getPath();
                    if(m_context == null)
                        m_context = getApplicationContext();
                    tryStartGame(m_context);
                }
            }
            else if(scheme.equals(ContentResolver.SCHEME_CONTENT))
            {
                Uri url = intent.getData();
                if(url != null)
                {
                    Log.d(LOG_TAG, "Got a content: " + url + ";");
                    filePathToOpen = url.getPath();
                    if(m_context == null)
                        m_context = getApplicationContext();
                    tryStartGame(m_context);
                }
            }
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

    public void updateOverlook()
    {
        SharedPreferences setup = PreferenceManager.getDefaultSharedPreferences(getBaseContext());
        String gameAssetsPath = setup.getString("setup_assets_path", "");

        String logoImagePath = "graphics/ui/MenuGFX2.png";
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
    }

    private boolean checkFilePermissions(int requestCode)
    {
        final int grant = PackageManager.PERMISSION_GRANTED;

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M)
        {
            final String exStorage = Manifest.permission.WRITE_EXTERNAL_STORAGE;
            if (ContextCompat.checkSelfPermission(this, exStorage) == grant) {
                Log.d(LOG_TAG, "File permission is granted");
            } else {
                Log.d(LOG_TAG, "File permission is revoked");
            }
        }

        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN)
        {
            final String exStorage = Manifest.permission.WRITE_EXTERNAL_STORAGE;
            if((ContextCompat.checkSelfPermission(this, exStorage) == grant))
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
        }

        return false;
    }

    public boolean hasManageAppFS()
    {
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.R && !Environment.isExternalStorageManager())
        {
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

        if(grantResults.length <= 0)
            return;
        if(!permissions[0].equals(Manifest.permission.WRITE_EXTERNAL_STORAGE))
            return;
        if(grantResults[0] != PackageManager.PERMISSION_GRANTED)
            return;
        if(!hasManageAppFS())
            return;

        if(requestCode == READWRITE_PERMISSION_FOR_GAME)
            tryStartGame(m_context);
    }

    public void startGame()
    {
        Intent myIntent = new Intent(Launcher.this, thextechActivity.class);
        if(!filePathToOpen.isEmpty())
            myIntent.putExtra("do-open-file", filePathToOpen);
//        myIntent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TASK);
//        myIntent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
        myIntent.addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP);
        Launcher.this.startActivity(myIntent);
        Launcher.this.finish();
    }
}