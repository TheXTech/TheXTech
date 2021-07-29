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
                    section = m.group(1).trim();
                }
                else if(section != null)
                {
                    m = _keyValue.matcher(line);
                    if(m.matches())
                    {
                        String key = m.group(1).trim();
                        String value = m.group(2).trim();
                        if(value.startsWith("\"") && value.endsWith("\""))
                            value = value.substring(1, value.length()-1).trim();
                        Map<String, String> kv = _entries.get(section);
                        if(kv == null)
                            _entries.put(section, kv = new HashMap<>());
                        kv.put(key, value);
                    }
                }
            }
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
    }

    public String getString(String section, String key, String defaultvalue)
    {
        Map<String, String> kv = _entries.get(section);
        if (kv == null)
        {
            return defaultvalue;
        }
        return kv.get(key);
    }

    public int getInt(String section, String key, int defaultvalue)
    {
        Map<String, String> kv = _entries.get(section);
        if (kv == null)
        {
            return defaultvalue;
        }
        return Integer.parseInt(kv.get(key));
    }

    public float getFloat(String section, String key, float defaultvalue)
    {
        Map<String, String> kv = _entries.get(section);
        if (kv == null)
        {
            return defaultvalue;
        }
        return Float.parseFloat(kv.get(key));
    }

    public double getDouble(String section, String key, double defaultvalue)
    {
        Map<String, String> kv = _entries.get(section);
        if (kv == null)
        {
            return defaultvalue;
        }
        return Double.parseDouble(kv.get(key));
    }
}