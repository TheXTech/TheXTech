package ru.wohlsoft.thextech;

/*
    Code made by Aerospace and published at the StackOverflow https://stackoverflow.com/a/15638381
    CC BY-SA 3.0
 */

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.HashMap;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class IniFile
{
    private final Pattern _section = Pattern.compile("\\s*\\[([^]]*)\\]\\s*");
    private final Pattern _keyValue = Pattern.compile("\\s*([^=]*)=(.*)");
    private final Map<String, Map<String, String>> _entries = new HashMap<>();

    public IniFile(String path)
    {
        load(path);
    }

    public void load(String path)
    {
        try
        {
            BufferedReader br = new BufferedReader(new FileReader(path));
            String line;
            String section = null;

            while((line = br.readLine()) != null)
            {
                Matcher m = _section.matcher(line);

                if(m.matches())
                {
                    section = m.group(1);
                    if(section != null)
                        section = section.trim();
                }
                else if(section != null)
                {
                    m = _keyValue.matcher(line);
                    if(m.matches())
                        insertValue(section, m);
                }
            }
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
    }

    private void insertValue(String section, Matcher m)
    {
        String key = m.group(1);
        String value = m.group(2);

        if(key == null || value == null)
            return;

        key = key.trim();
        value = value.trim();

        boolean quoteOpen = false;
        for(int i = 0; i < value.length(); ++i)
        {
            char c = value.charAt(i);
            if(!quoteOpen && c == '"')
            {
                quoteOpen = true;
            }
            else if(quoteOpen && c == '"')
            {
                quoteOpen = false;
            }
            else if(!quoteOpen && c == ';')
            {
                value = value.substring(0, i).trim();
                break; // Found leading comment, let's strip it!
            }
        }

        if(value.startsWith("\"") && value.endsWith("\""))
            value = value.substring(1, value.length()-1).trim();

        Map<String, String> kv = _entries.get(section);

        if(kv == null)
            _entries.put(section, kv = new HashMap<>());

        kv.put(key, value);
    }

    public String getString(String section, String key, String defaultvalue)
    {
        Map<String, String> kv = _entries.get(section);
        if(kv == null)
            return defaultvalue;
        String ret = kv.get(key);
        if(ret == null)
            return defaultvalue;
        return ret;
    }

    public int getInt(String section, String key, int defaultvalue)
    {
        Map<String, String> kv = _entries.get(section);
        if(kv == null)
            return defaultvalue;
        String ret = kv.get(key);
        if(ret == null)
            return defaultvalue;
        return Integer.parseInt(ret);
    }

    public float getFloat(String section, String key, float defaultvalue)
    {
        Map<String, String> kv = _entries.get(section);
        if(kv == null)
            return defaultvalue;
        String ret = kv.get(key);
        if(ret == null)
            return defaultvalue;
        return Float.parseFloat(ret);
    }

    public double getDouble(String section, String key, double defaultvalue)
    {
        Map<String, String> kv = _entries.get(section);
        if(kv == null)
            return defaultvalue;
        String ret = kv.get(key);
        if(ret == null)
            return defaultvalue;
        return Double.parseDouble(ret);
    }
}
