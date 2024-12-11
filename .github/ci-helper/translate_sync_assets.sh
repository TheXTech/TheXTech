#!/bin/bash

OLD=$PWD
GIT_ROOT=$PWD/build-git
WORKDIR="$PWD/.github/ci-helper"

mkdir -p build-git

# FIXME: Replace this hardcoded list with a server-side one to don't re-commit this list
# every time
for q in \
    "git@gitea.wohlsoft.ru:Games-for-TheXTech/thextech-smbx.git" \
    "git@gitea.wohlsoft.ru:Games-for-TheXTech/thextech-adventures-of-demo.git" \
    "git@gitea.wohlsoft.ru:Games-for-TheXTech/thextech-convert-kit.git" \
    "git@gitea.wohlsoft.ru:Games-for-TheXTech/thextech-a2xt-analog-funk.git" \
    "git@gitea.wohlsoft.ru:Games-for-TheXTech/thextech-a2xt-prelude-to-the-stupid.git" \
    "git@gitea.wohlsoft.ru:Games-for-TheXTech/thextech-lowser-s-conquest-beta.git" \
    "git@gitea.wohlsoft.ru:Games-for-TheXTech/thextech-sarasaland-adventure-v1-2.git" \
    "git@gitea.wohlsoft.ru:Games-for-TheXTech/thextech-sarasaland-adventure-2.git" \
    "git@gitea.wohlsoft.ru:Games-for-TheXTech/thextech-smbx-nes.git" \
    "git@gitea.wohlsoft.ru:Games-for-TheXTech/thextech-smbx-nostalgic.git" \
    "git@gitea.wohlsoft.ru:Games-for-TheXTech/thextech-super-talking-time-bros-1n2-v1-5.git" \
    "git@gitea.wohlsoft.ru:Games-for-TheXTech/thextech-super-talking-time-bros-2-5-v1-2.git" \
    ;
do
    echo "--------------- $q ---------------"
    cd "${GIT_ROOT}"
    git clone "$q" --depth 1 repo

    cd "${WORKDIR}"
    bash translate_update.sh "${GIT_ROOT}/repo"

    cd "${GIT_ROOT}/repo"
    if [[ ! -z $(git status -s) ]]; then
        echo "-- Found updated languages, commiting..."
        git add --all
        git commit --author="${GIT_AUTHOR}" -m "Synchronized translations"
        git push
    else
        echo "-- No updates found, skipping..."
    fi
    cd ..
    rm -Rf repo
done

cd "${OLD}"
