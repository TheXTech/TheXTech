#!/bin/bash

OLD=$PWD
GIT_ROOT=$PWD/build-git
WORKDIR="$PWD/.github/ci-helper"

git clone https://github.com/TheXTech/TheXTech.git --depth 1 -b stable-1.3.7.x build-git

cp -av resources/languages/* build-git/resources/languages/

cd build-git
if [[ ! -z $(git status -s) ]]; then
    echo "-- Found updated languages, commiting..."
    git add --all
    git commit --author="${GIT_AUTHOR}" -m "Synchronized translations"
    git push
else
    echo "-- No updates found, skipping..."
fi

cd ..
rm -Rf build-git

cd "${OLD}"

exit 0
