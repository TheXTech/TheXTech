#!/bin/bash

RUNNER_OS=$1
SUBDIR_NAME=$2
EXECUTABLE_NAME=$3
ARCHIVE_NAME=$4
ASSETS_NAME=$5


cd build

mkdir -p "package/${SUBDIR_NAME}"

cp ../changelog.txt "package/${SUBDIR_NAME}/"

cp ../LICENSE "package/${SUBDIR_NAME}/License.TheXTech.txt"

cp ../README.md "package/${SUBDIR_NAME}/ReadMe.txt"
cp ../README.RUS.md "package/${SUBDIR_NAME}/ReadMe.RUS.txt"
cp ../README.ESP.md "package/${SUBDIR_NAME}/ReadMe.ESP.txt"

if [[ "${RUNNER_OS}" == "Windows" ]]; then
    cp output/bin/thextech.exe "package/${SUBDIR_NAME}/${EXECUTABLE_NAME}.exe"
    cp output/bin/*.dll "package/${SUBDIR_NAME}/"
elif [[ "${RUNNER_OS}" == "Linux" ]]; then
    cp output/bin/thextech "package/${SUBDIR_NAME}/${EXECUTABLE_NAME}"
fi

if [[ "${ASSETS_NAME}" != "none" ]]; then
    cp -r ../${ASSETS_NAME}/* "package/${SUBDIR_NAME}/"
fi

cd package

if [[ "${RUNNER_OS}" == "Windows" ]]; then
    7z a "${ARCHIVE_NAME}.7z" "${SUBDIR_NAME}"
else
    tar -cvzf "${ARCHIVE_NAME}.tar.gz" "${SUBDIR_NAME}"
fi

rm -Rf "${SUBDIR_NAME}"

cd ../..
