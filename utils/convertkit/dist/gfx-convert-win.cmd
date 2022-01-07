@echo off

echo ------------------------------------------------------------------------
echo    All default masked GIF stuff will be converted into PNG
echo ------------------------------------------------------------------------
echo  Do you want to remove all old GIFs [Y]? or keep both GIFs and PNG [N]?
echo ------------------------------------------------------------------------
choice
if errorlevel 2 set REMOVE=
if errorlevel 1 set REMOVE=-r

set smbxexe=smbx.exe
if exist asmbxt.exe set smbxexe=asmbxt.exe
if exist a2mbxt.exe set smbxexe=a2mbxt.exe

echo Executing exe2ui %smbxexe% ...
exe2ui.exe %smbxexe%

echo Executing GIFs2PNG.exe -d %REMOVE% graphics ...
GIFs2PNG.exe -d %REMOVE% graphics

echo Cleaning up from garbage...
del /s graphics\block\*block-*.txt > NUL
del /s graphics\background\*background-*.txt > NUL
del /s graphics\npc\*npc-*.txt > NUL

echo .
echo .
echo ---------------------------------------------------------------
echo   Press any key to quit...
echo ---------------------------------------------------------------
pause > NUL
