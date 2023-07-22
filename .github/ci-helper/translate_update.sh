#!/bin/bash

OLD_DIR=~+

ASSETS_ROOT=$1
REPO_ROOT="$PWD/../../resources/languages"

LANGS_IN=$1/languages/patches
LANGS_OUT=$1/languages

cd "${LANGS_IN}"

for q in assets_*.json; do
    printf "\n\n---------------------------------------------\nFile: %s\n" $q
    echo "===== copy  ====="
    GENERIC_TR=thextech_${q:7:-5}.json
    cp -v "${REPO_ROOT}/${GENERIC_TR}" "${LANGS_OUT}/${GENERIC_TR}"
    echo "===== patch ====="
    echo python3 "${OLD_DIR}/translate_patcher.py" "${REPO_ROOT}/$q" "${LANGS_IN}/$q" "${LANGS_OUT}/$q"
    python3 "${OLD_DIR}/translate_patcher.py" "${REPO_ROOT}/$q" "${LANGS_IN}/$q" "${LANGS_OUT}/$q"
    echo "---------------------------------------------"
done

cd "$OLD_DIR"
