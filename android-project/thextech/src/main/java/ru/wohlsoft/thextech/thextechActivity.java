package ru.wohlsoft.thextech;

import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.widget.LinearLayout;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;

import org.libsdl.app.SDLActivity;

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
    protected String[] getLibraries()
    {
        return new String[] {
            "SDL2",
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

        // Detect current language of the system
        String lang = detectLanguage();
        // TODO: NOT IMPLEMENTED YET
//        if(lang.length() >= 1)
//            args.add("--lang=" + lang);

        String[] argsOut = new String[args.size()];
        args.toArray(argsOut);

        return argsOut;
    }

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        // Not needed yet
//        LinearLayout ll = new LinearLayout(this);
//        ll.setBackground(getResources().getDrawable(R.mipmap.buttons));
//        addContentView(ll, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT,
//                LinearLayout.LayoutParams.MATCH_PARENT));
        this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
    }

    public static native void setKeyPos(int cmd, float left, float top, float right, float bottom);
    public static native void setCanvasSize(float width, float height);

}
