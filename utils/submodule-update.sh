#!/bin/bash

reponame=$(basename `git rev-parse --show-toplevel`)
echo "-----------------------------------"
echo "Updating repo $reponame"
if [[ "$reponame" == "LuaJIT" ]]; then
    echo "Branch v2.1"
    git checkout v2.1
    git pull origin v2.1
elif [[ "$reponame" == "angle-shader-translator" ]]; then
    echo "Branch dist-no-spirv"
    git checkout dist-no-spirv
    git pull origin dist-no-spirv
elif [[ "$reponame" == "harfbuzz" || "$reponame" == "thextech-discord-rpc" ]]; then
    echo "Branch main"
    git checkout main
    git pull origin main
# ========================================================
# TEMPORARY, REMOVE THIS ONCE THIS GETS MERGED INTO MASTER
elif [[ "$reponame" == "PGE_File_Formats" ]]; then
    echo "Branch wip-mdx"
    git checkout wip-mdx
    git pull origin wip-mdx
# ========================================================
elif [[ "$reponame" == "mbediso" ]]; then
    echo "Branch main"
    git checkout main
    git pull origin main
    # Sync submodules
    git submodule init
    git submodule update
else
    echo "Branch master"
    git checkout master
    git pull origin master
fi
