package ru.wohlsoft.thextech;


import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Point;
import android.graphics.Rect;
import android.graphics.drawable.Drawable;
//import android.os.Build;
import android.os.Environment;
import android.util.DisplayMetrics;
import android.util.TypedValue;
import android.view.*;
import android.widget.*;

import java.io.File;
import java.io.FilenameFilter;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.*;

import androidx.core.content.res.ResourcesCompat;


/**
 * Created with IntelliJ IDEA.
 * User: Scogun
 * Date: 27.11.13
 * Time: 10:47
 */
public class OpenFileDialog extends AlertDialog.Builder {

    private String currentPath = Environment.getExternalStorageDirectory().getPath();
    private String initialPath = currentPath;
    private List<File> files = new ArrayList<File>();
    private TextView title;
    private ListView listView;
    private FilenameFilter filenameFilter;
    private int selectedIndex = -1;
    private OpenDialogListener listener;
    private Drawable folderIcon;
    private Drawable fileIcon;
    private String accessDeniedMessage;
    private Boolean m_directoryMode = false;

    public interface OpenDialogListener
    {
        public void OnSelectedFile(Context ctx, String fileName, String lastPath);
        public void OnSelectedDirectory(Context ctx, String lastPath);
    }

    private class FileAdapter extends ArrayAdapter<File> {

        public FileAdapter(Context context, List<File> files) {
            super(context, android.R.layout.simple_list_item_1, files);
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            TextView view = (TextView) super.getView(position, convertView, parent);
            File file = getItem(position);
            assert file != null;
            view.setText(file.getName());
            if (file.isDirectory()) {
                setDrawable(view, folderIcon);
                view.setTextColor(Color.DKGRAY);
                view.setBackgroundColor(Color.LTGRAY);
            } else {
                setDrawable(view, fileIcon);
                view.setTextColor(Color.BLACK);
                if (selectedIndex == position)
                    view.setBackgroundColor(Color.LTGRAY);
                else
                    view.setBackgroundColor(Color.WHITE);
            }
            return view;
        }

        private void setDrawable(TextView view, Drawable drawable) {
            if (view != null) {
                if (drawable != null) {
                    drawable.setBounds(0, 0, 60, 60);
                    view.setCompoundDrawables(drawable, null, null, null);
                } else {
                    view.setCompoundDrawables(null, null, null, null);
                }
            }
        }
    }

    public OpenFileDialog(Context context) {
        super(context);
        title = createTitle(context);
        changeTitle();

        LinearLayout linearLayout = createMainLayout(context);

        LinearLayout menuBar = createMenuLayout(linearLayout.getContext());
        menuBar.addView(createBackItem(context));
        menuBar.addView(createGoHomeItem(context));
        menuBar.addView(createGoInitPathItem(context));
        linearLayout.addView(menuBar);

        listView = createListView(context);
        linearLayout.addView(listView);

        setCustomTitle(title)
            .setView(linearLayout)
            .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener()
            {
                @Override
                public void onClick(DialogInterface dialog, int which)
                {
                    if(m_directoryMode)
                    {
                        listener.OnSelectedDirectory(getContext(), currentPath);
                    }
                    else if (selectedIndex > -1 && listener != null)
                    {
                        listener.OnSelectedFile(getContext(), listView.getItemAtPosition(selectedIndex).toString(), currentPath);
                    }
                }
            })
            .setNegativeButton(android.R.string.cancel, null);
    }

    @Override
    public AlertDialog show() {
        files.addAll(getFiles(currentPath));
        listView.setAdapter(new FileAdapter(getContext(), files));
        return super.show();
    }

    public OpenFileDialog setFilter(final String filter) {
        filenameFilter = new FilenameFilter() {
            @Override
            public boolean accept(File file, String fileName) {
                Pattern filt = Pattern.compile(filter, Pattern.CASE_INSENSITIVE);
                File tempFile = new File(String.format("%s/%s", file.getPath(), fileName));
                if (tempFile.isFile()) {
                    Matcher m = filt.matcher(tempFile.getName());
                    return m.matches();
                }

                return true;
            }
        };
        return this;
    }

    public OpenFileDialog setDirectoryMode()
    {
        m_directoryMode = true;
        return this;
    }

    public OpenFileDialog setCurrentDirectory(String path)
    {
        currentPath = path;
        initialPath = path;
        changeTitle();
        return this;
    }

    public OpenFileDialog setOpenDialogListener(OpenDialogListener listener) {
        this.listener = listener;
        return this;
    }

    public OpenFileDialog setFolderIcon(Drawable drawable) {
        this.folderIcon = drawable;
        return this;
    }

    public OpenFileDialog setFileIcon(Drawable drawable) {
        this.fileIcon = drawable;
        return this;
    }

    public OpenFileDialog setAccessDeniedMessage(String message) {
        this.accessDeniedMessage = message;
        return this;
    }

    private static Display getDefaultDisplay(Context context) {
        return ((WindowManager) context.getSystemService(Context.WINDOW_SERVICE)).getDefaultDisplay();
    }

    private static Point getScreenSize(Context context) {
        Point screeSize = new Point();
//        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.HONEYCOMB_MR2) {
        getDefaultDisplay(context).getSize(screeSize);
//        } else {
//            screeSize.set( getDefaultDisplay(context).getWidth(), getDefaultDisplay(context).getHeight() );
//        }
        return screeSize;
    }

    private static int getLinearLayoutMinHeight(Context context) {
        return getScreenSize(context).y;
    }

    private LinearLayout createMainLayout(Context context) {
        LinearLayout linearLayout = new LinearLayout(context);
        linearLayout.setOrientation(LinearLayout.VERTICAL);
        linearLayout.setMinimumHeight(getLinearLayoutMinHeight(context));
        return linearLayout;
    }

    private LinearLayout createMenuLayout(Context context) {
        LinearLayout linearLayout = new LinearLayout(context);
        linearLayout.setOrientation(LinearLayout.HORIZONTAL);
        return linearLayout;
    }

    private int getItemHeight(Context context) {
        TypedValue value = new TypedValue();
        DisplayMetrics metrics = new DisplayMetrics();
        context.getTheme().resolveAttribute(android.R.attr.listPreferredItemHeightSmall, value, true);
        getDefaultDisplay(context).getMetrics(metrics);
        return (int) TypedValue.complexToDimension(value.data, metrics);
    }

    private TextView createTextView(Context context, int style) {
        TextView textView = new TextView(context);
        //textView.setTextAppearance(context, style);
        int itemHeight = getItemHeight(context);
        textView.setLayoutParams(new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, itemHeight));
        textView.setMinHeight(itemHeight);
        textView.setGravity(Gravity.CENTER_VERTICAL);
        textView.setPadding(15, 0, 0, 0);
        return textView;
    }

    private TextView createTitle(Context context) {
        return createTextView(context, android.R.style.TextAppearance_DeviceDefault_DialogWindowTitle);
    }

    private TextView createBackItem(Context context) {
        TextView textView = createTextView(context, android.R.style.TextAppearance_DeviceDefault_Small);
        Drawable drawable = ResourcesCompat.getDrawable(getContext().getResources(), R.drawable.ic_menu_back, context.getTheme());
        assert drawable != null;
        drawable.setBounds(0, 0, 60, 60);
        textView.setCompoundDrawables(drawable, null, null, null);
        textView.setLayoutParams(new ViewGroup.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        textView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                File file = new File(currentPath);
                File parentDirectory = file.getParentFile();
                if (parentDirectory != null) {
                    currentPath = parentDirectory.getPath();
                    RebuildFiles(((FileAdapter) listView.getAdapter()));
                }
            }
        });
        return textView;
    }

    private TextView createGoHomeItem(Context context) {
        TextView textView = createTextView(context, android.R.style.TextAppearance_DeviceDefault_Small);
        Drawable drawable = ResourcesCompat.getDrawable(getContext().getResources(), R.drawable.ic_menu_home, context.getTheme());
        assert drawable != null;
        drawable.setBounds(0, 0, 60, 60);
        textView.setCompoundDrawables(drawable, null, null, null);
        textView.setLayoutParams(new ViewGroup.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        textView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                currentPath = Environment.getExternalStorageDirectory().getPath();
                RebuildFiles(((FileAdapter) listView.getAdapter()));
            }
        });
        return textView;
    }

    private TextView createGoInitPathItem(Context context) {
        TextView textView = createTextView(context, android.R.style.TextAppearance_DeviceDefault_Small);
        Drawable drawable = ResourcesCompat.getDrawable(getContext().getResources(), android.R.drawable.ic_menu_directions, context.getTheme());
        assert drawable != null;
        drawable.setBounds(0, 0, 60, 60);
        textView.setCompoundDrawables(drawable, null, null, null);
        textView.setLayoutParams(new ViewGroup.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT));
        textView.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                currentPath = initialPath;
                RebuildFiles(((FileAdapter) listView.getAdapter()));
            }
        });
        return textView;
    }

    private int getTextWidth(String text, Paint paint) {
        Rect bounds = new Rect();
        paint.getTextBounds(text, 0, text.length(), bounds);
        return bounds.left + bounds.width() + 80;
    }

    private void changeTitle() {
        String titleText = currentPath;
        int screenWidth = getScreenSize(getContext()).x;
        int maxWidth = (int) (screenWidth * 0.99);
        if (getTextWidth(titleText, title.getPaint()) > maxWidth) {
            while (getTextWidth("..." + titleText, title.getPaint()) > maxWidth) {
                int start = titleText.indexOf("/", 2);
                if (start > 0)
                    titleText = titleText.substring(start);
                else
                    titleText = titleText.substring(2);
            }
            title.setText( String.format("...%s", titleText) );
        } else {
            title.setText(titleText);
        }
    }

    private List<File> getFiles(String directoryPath) {
        File directory = new File(directoryPath);
        File[] list = directory.listFiles(filenameFilter);
        if(list == null)
            list = new File[]{};
        List<File> fileList = Arrays.asList(list);
        Collections.sort(fileList, new Comparator<File>() {
            @Override
            public int compare(File file, File file2) {
                if (file.isDirectory() && file2.isFile())
                    return -1;
                else if (file.isFile() && file2.isDirectory())
                    return 1;
                else
                    return file.getPath().compareToIgnoreCase(file2.getPath());
            }
        });
        return fileList;
    }

    private void RebuildFiles(ArrayAdapter<File> adapter) {
        try {
            List<File> fileList = getFiles(currentPath);
            files.clear();
            selectedIndex = -1;
            files.addAll(fileList);
            adapter.notifyDataSetChanged();
            changeTitle();
        } catch (NullPointerException e) {
            String message = getContext().getResources().getString(android.R.string.unknownName);
            if (!accessDeniedMessage.equals(""))
                message = accessDeniedMessage;
            Toast.makeText(getContext(), message, Toast.LENGTH_SHORT).show();
        }
    }

    private ListView createListView(Context context) {
        ListView listView = new ListView(context);
        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {

            @Override
            public void onItemClick(AdapterView<?> adapterView, View view, int index, long l) {
                final ArrayAdapter<File> adapter = (FileAdapter) adapterView.getAdapter();
                File file = adapter.getItem(index);
                assert file != null;
                if (file.isDirectory()) {
                    currentPath = file.getPath();
                    RebuildFiles(adapter);
                } else {
                    if (index != selectedIndex)
                        selectedIndex = index;
                    else
                        selectedIndex = -1;
                    adapter.notifyDataSetChanged();
                }
            }
        });
        return listView;
    }
}
