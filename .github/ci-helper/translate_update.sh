#!/bin/bash

OLD_DIR=~+

ASSETS_ROOT=$1

LANGS_IN="$PWD/../../resources/languages"
LANGS_PATCH=$1/languages/patches
LANGS_OUT=$1/languages

# Update only existing files at the target (Don't copy new files, they should be placed manually to add them for the autosync)
cd "${LANGS_OUT}"

function update_tr_file()
{
    q="$1"
    if [[ ! -f "${LANGS_PATCH}/$q" ]]; then
        # If no patch file exists, just copy
        printf "==== COPY: %s ====\n" $q
        cp -v "${LANGS_IN}/$q" "${LANGS_OUT}/$q"
    else
        # Otherwise, apply the patch
        printf "==== PATCH: %s ====\n" $q
        echo python3 "${OLD_DIR}/translate_patcher.py" "${LANGS_IN}/$q" "${LANGS_PATCH}/$q" "${LANGS_OUT}/$q"
        python3 "${OLD_DIR}/translate_patcher.py" "${LANGS_IN}/$q" "${LANGS_PATCH}/$q" "${LANGS_OUT}/$q"
    fi
    printf "\n"
}

echo "---------------------------------------------"

for q in thextech_*.json; do
    update_tr_file "$q"
done

for q in assets_*.json; do
    update_tr_file "$q"
done

echo "---------------------------------------------"

cd "$OLD_DIR"
