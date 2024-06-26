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
elif [[ "$reponame" == "libvpx" ]]; then
    echo "Branch stable-moondust"
    git checkout stable-moondust
    git pull origin stable-moondust
elif [[ "$reponame" == "ffmpeg" ]]; then
    echo "Branch release/7.0"
    git checkout "release/7.0"
    git pull origin "release/7.0"
elif [[ "$reponame" == "harfbuzz" || "$reponame" == "thextech-discord-rpc" ]]; then
    echo "Branch main"
    git checkout main
    git pull origin main
else
    echo "Branch master"
    git checkout master
    git pull origin master
fi
