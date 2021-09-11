#!/bin/bash

echo "------------------------------------------------------------------------"
echo "   All default masked GIF stuff will be converted into PNG"
echo "------------------------------------------------------------------------"
echo " Do you want to remove all old GIFs [Y]? or keep both GIFs and PNG [N]?"
echo "------------------------------------------------------------------------"
echo "Answer Y/N ?"
read -n 1 REMOVE

echo -e "\n\n"

if [[ "$REMOVE" == "y" ]]; then
    REMOVE=-r
else
    REMOVE=
fi

smbxexe="smbx.exe"
if [[ -f "asmbxt.exe" ]]; then
    smbxexe="asmbxt.exe"
elif [[ -f "a2mbxt.exe" ]]; then
    smbxexe="a2mbxt.exe"
fi

chmod +x exe2ui-linux-x86_64
echo "Executing exe2ui ${smbxexe}..."
./exe2ui-linux-x86_64 "${smbxexe}"

chmod +x GIFs2PNG-linux-x86_64
echo "Executing ./GIFs2PNG-linux-x86_64 -d $REMOVE graphics ..."
./GIFs2PNG-linux-x86_64 -d $REMOVE graphics

echo "Cleaning up from garbage..."
find graphics -name "npc*.txt" -delete
find graphics -name "Thumbds.db" -delete

echo -e "\n\n"
echo "---------------------------------------------------------------"
echo "  Press any key to quit..."
echo "---------------------------------------------------------------"
read -n 1
