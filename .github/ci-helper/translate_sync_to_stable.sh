#!/bin/bash

OLD=$PWD
GIT_ROOT=$PWD/build-git
WORKDIR="$PWD/.github/ci-helper"

STABLE_BRANCH=stable-1.3.7.x

if [[ -z "$GITHUB_TOKEN" ]]; then
    echo 'Missing input "github_token: $GITHUB_TOKEN".';
    exit 1;
fi

git clone https://github.com/TheXTech/TheXTech.git --depth 1 -b ${STABLE_BRANCH} build-git

cp -av resources/languages/* build-git/resources/languages/

cd build-git
if [[ ! -z $(git status -s) ]]; then
    echo "-- Found updated languages, commiting..."
    git add --all
    git commit --author="${GIT_AUTHOR}" -m "Synchronized translations with mainstream branch"
    remote_repo="https://${GITHUB_ACTOR}:${GITHUB_TOKEN}@github.com/TheXTech/TheXTech.git"
    git push "${remote_repo}" ${STABLE_BRANCH}
else
    echo "-- No updates found, skipping..."
fi

cd ..
rm -Rf build-git

cd "${OLD}"

exit 0
