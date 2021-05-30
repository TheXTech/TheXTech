#include <cstdio>
#include <string>

void fwritestr(FILE* f, const std::string& s)
{
    char buf[512];
    int dest = 0;
    for (const char& c : s)
    {
        if(dest == 512)
            break;
        if(c == '\t' || c == '\r' || c == '\n' || c == '"')
            continue;
        buf[dest] = c;
        dest ++;
    }
    buf[dest] = '\0';
    fprintf(f, "\"%s\"\r\n", buf);
}

void fwritestr_multiline(FILE* f, const std::string& s)
{
    char buf[512];
    int dest = 0;
    for (const char& c : s)
    {
        if(dest == 512)
            break;
        if(c == '\t' || c == '\r')
            continue;
        if(c == '"')
            buf[dest] = '\'';
        else if(c == '\n')
        {
            if(dest == 511) break;
            buf[dest] = '\r';
            dest ++;
            buf[dest] = '\n';
        }
        else
            buf[dest] = c;
        dest ++;
    }
    buf[dest] = '\0';
    fprintf(f, "\"%s\"\r\n", buf);
}

void fwritenum(FILE* f, int n)
{
    fprintf(f, "%d\r\n", n);
}

void fwritebool(FILE* f, bool b)
{
    if (b)
        fprintf(f, "#TRUE#\r\n");
    else
        fprintf(f, "#FALSE#\r\n");
}

void fwritefloat(FILE* f, float n)
{
    fprintf(f, "%f\r\n", n);
}
