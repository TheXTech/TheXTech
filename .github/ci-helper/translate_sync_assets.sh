#!/bin/bash

OLD=$PWD
GIT_ROOT=$PWD/build-git
WORKDIR="$PWD/.github/ci-helper"

mkdir -p build-git

function update_repo()
{
    q=$1
    branch=$2

    if [[ "$branch" != "" ]]; then
        branch_command=-b $branch
    fi

    echo "--------------- $q ---------------"
    cd "${GIT_ROOT}"
    git clone "$q" --depth 1 $branch_command repo

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
}

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
    update_repo "$q"
done

# Nostalgic Paradise needs another branch
update_repo git@gitea.wohlsoft.ru:Games-for-TheXTech/nostalgic-paradise.git development

cd "${OLD}"
