#!/bin/bash

# $1 assets; $2 icon $3 bundle name

ASSETS_NAME=$1
ICON_FILE=$2
BUNDLE_NAME=$3
ARCHIVE_NAME=$4

if [[ "${ASSETS_NAME}" != "none" ]]; then
    echo "Preparing the application..."
    cp -R TheXTech.app "tmpapp"
    cp -R $ASSETS_NAME/* "tmpapp/Contents/Resources/assets/"
    find tmpapp -name ".DS_Store" -delete
    plutil -replace CFBundleName -string "$BUNDLE_NAME" "tmpapp/Contents/Info.plist"
    plutil -replace CFBundleIconFile -string "$ICON_FILE" "tmpapp/Contents/Info.plist"
    mv tmpapp "$BUNDLE_NAME.app"
fi

mkdir dmg-root

cp LICENSE "dmg-root/License.TheXTech.txt"

cp README.md "dmg-root/ReadMe.txt"
cp README.RUS.md "dmg-root/ReadMe.RUS.txt"
cp README.ESP.md "dmg-root/ReadMe.ESP.txt"

echo "== mv \"$BUNDLE_NAME.app\" dmg-root/"
mv "$BUNDLE_NAME.app" dmg-root/

if [[ "${ASSETS_NAME}" == "none" ]]; then
    echo "Creating ZIP..."
    cd dmg-root
    zip -9 -r ../${ARCHIVE_NAME} *
    cd ..
else
    echo "Creating DMG..."
    ./.github/ci-helper/create-dmg.sh \
        --volname "$BUNDLE_NAME" \
        --window-size 800 600 \
        --app-drop-link 450 320 \
        --no-internet-enable \
        "$ARCHIVE_NAME" \
        "dmg-root/"
fi

echo "Cleaning up..."
rm -Rf dmg-root
printf "\n----------------------------------------------------------------\n\n"
