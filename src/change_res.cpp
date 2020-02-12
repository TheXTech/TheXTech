#include "globals.h"
#include "change_res.h"

// TODO: Instead of this CRAP, get SDL2 usage

// Get current settings
const long ENUM_CURRENT_SETTINGS = -1;
// Pixels in width
const long DM_PELSWIDTH = 0x80000;
// Pixels in height
const long DM_PELSHEIGHT = 0x100000;
        // Color Depth
const long DM_BITSPERPEL = 0x40000;
// Display Frequency
const long DM_DISPFREQ = 0x400000;
const long CDS_TEST = 0x4;

//Private Type DEVMODE
struct DEVMODE
{
//    dmDeviceName As String * 32 'Name of graphics card?????
    std::string dmDeviceName;
//    dmSpecVersion As Integer
    int dmSpecVersion;
//    dmDriverVersion As Integer 'graphics card driver version?????
    int dmDriverVersion;
//    dmSize As Integer
    int dmSize;
//    dmDriverExtra As Integer
    int dmDriverExtra;
//    dmFields As Long
    int dmFields;
//    dmOrientation As Integer
    int dmOrientation;
//    dmPaperSize As Integer
    int dmPaperSize;
//    dmPaperLength As Integer
    int dmPaperLength;
//    dmPaperWidth As Integer
    int dmPaperWidth;
//    dmScale As Integer
    int dmScale;
//    dmCopies As Integer
    int dmCopies;
//    dmDefaultSource As Integer
    int dmDefaultSource;
//    dmPrintQuality As Integer
    int dmPrintQuality;
//    dmColor As Integer
    int dmColor;
//    dmDuplex As Integer
    int dmDuplex;
//    dmYResolution As Integer
    int dmYResolution;
//    dmTTOption As Integer
    int dmTTOption;
//    dmCollate As Integer
    int dmCollate;
//    dmFormName As String * 32 'Name of form?????
    std::string dmFormName;
//    dmUnusedPadding As Integer
    int dmUnusedPadding;
//    dmBitsPerPel As Integer 'Color Quality (can be 8, 16, 24, 32 or even 4)
    int dmBitsPerPel;
//    dmPelsWidth As Long 'Display Width in pixels
    int dmPelsWidth;
//    dmPelsHeight As Long 'Display height in pixels
    int dmPelsHeight;
//    dmDisplayFlags As Long
    int dmDisplayFlags;
//    dmDisplayFrequency As Long 'Display frequency
    int dmDisplayFrequency;
//    dmICMMethod As Long 'NT 4.0
    int dmICMMethod;
//    dmICMIntent As Long 'NT 4.0
    int dmICMIntent;
//    dmMediaType As Long 'NT 4.0
    int dmMediaType;
//    dmDitherType As Long 'NT 4.0
    int dmDitherType;
//    dmReserved1 As Long 'NT 4.0
    int dmReserved1;
//    dmReserved2 As Long 'NT 4.0
    int dmReserved2;
//    dmPanningWidth As Long 'Win2000
    int dmPanningWidth;
//    dmPanningHeight As Long 'Win2000
    int dmPanningHeight;
//End Type
};

void SaveIt(int ScX, int ScY, int ScC, int ScF, std::string ScreenChanged);

DEVMODE DevM;
int OldX, OldY, OldColor, OldFreq;
int SetX, SetY, SetColor, SetFreq;

void GetCurrentRes()
{

}

void SetOrigRes()
{
    frmMain.setFullScreen(false);
    resChanged = false;
}

void ChangeRes(int, int, int, int)
{
    frmMain.setFullScreen(true);
}

void SaveIt(int ScX, int ScY, int ScC, int ScF, std::string ScreenChanged)
{

}
