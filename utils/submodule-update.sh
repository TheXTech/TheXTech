#!/bin/bash

reponame=$(basename `git rev-parse --show-toplevel`)
echo "-----------------------------------"
echo "Updating repo $reponame"
if [[ "$reponame" == "LuaJIT" ]]; then
    echo "Branch v2.1"
    git checkout v2.1
    git pull origin v2.1
elif [[ "$reponame" == "harfbuzz" ]]; then
    echo "Branch main"
    git checkout main
    git pull origin main
else
    echo "Branch master"
    git checkout master
    git pull origin master
fi
