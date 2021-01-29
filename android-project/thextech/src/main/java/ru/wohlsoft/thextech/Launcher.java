package ru.wohlsoft.thextech;

import android.Manifest;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import androidx.annotation.NonNull;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;

public class Launcher extends AppCompatActivity
{
    final String LOG_TAG = "TheXTech";
    public static final int READWRITE_PERMISSION_FOR_GAME = 1;

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
    }

    private void initUiSetup()
    {
        Button startGame = (Button) findViewById(R.id.startGame);
        startGame.setOnClickListener(new View.OnClickListener()
        {
            @Override
            public void onClick(View view)
            {
                OnStartGameClick(view);
            }
        });

        Button gameSettings = (Button) findViewById(R.id.gameSettings);
        gameSettings.setOnClickListener(
        new View.OnClickListener()
        {
            @Override
            public void onClick(View view)
            {
//                AlertDialog.Builder b = new AlertDialog.Builder(Launcher.this);
//                b.setTitle("WIP");
//                b.setMessage("Not implemented yet, sorry");
//                b.setNegativeButton(android.R.string.ok, null);
//                b.show();
                Intent myIntent = new Intent(Launcher.this, GameSettings.class);
                Launcher.this.startActivity(myIntent);
            }
        });
    }

    public void OnStartGameClick(View view)
    {
        // Here, thisActivity is the current activity
        if(checkFilePermissions(READWRITE_PERMISSION_FOR_GAME))
            return;
        startGame();
    }

    private boolean checkFilePermissions(int requestCode)
    {
        final int grant = PackageManager.PERMISSION_GRANTED;

        if (Build.VERSION.SDK_INT >= 23)
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

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults)
    {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults);
        if(grantResults.length > 0 && permissions[0].equals(Manifest.permission.WRITE_EXTERNAL_STORAGE) && grantResults[0] == PackageManager.PERMISSION_GRANTED)
        {
            if(requestCode == READWRITE_PERMISSION_FOR_GAME)
            {
                startGame();
            }
        }
    }

    public void startGame()
    {
        Intent myIntent = new Intent(Launcher.this, thextechActivity.class);
//        myIntent.putExtra("key", value); //Optional parameters
        Launcher.this.startActivity(myIntent);
        Launcher.this.finish();
    }
}