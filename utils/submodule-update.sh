#!/bin/bash

projroot=$(dirname "$1")
projroot_len=$((${#projroot}+1))
cur_dir=$(pwd)
repo_path=${cur_dir:$projroot_len}

echo "-----------------------------------"
# echo "Debug: projroot=$projroot"
# echo "Debug: projroot_len=$projroot_len"
# echo "Debug: cur_dir=$cur_dir"
# echo "Debug: repo_path=$repo_path"

dstbranch=""
found=false

while IFS= read -r line; do
    line_clear=$(echo "$line" | sed 's/^[ \t]*//g')

    if $found ; then
        if [[ "$line_clear" == "branch = "* ]]; then
            dstbranch=$(echo "$line_clear" | sed 's/^branch = *//g')
            echo "Found branch name: $dstbranch"
            break
        fi
    elif [[ "$line_clear" == "path = $repo_path" ]]; then
        found=true
    fi
done < "$1"

reponame=$(basename `git rev-parse --show-toplevel`)
echo "Updating repo $reponame (path=$repo_path), branch $dstbranch"

git checkout $dstbranch
git pull origin $dstbranch

if [[ -f .gitmodules ]]; then
    echo "========================================="
    echo "Running recursive submodule sync..."
    echo "========================================="
    git submodule init
    git submodule update
    echo ""
    git submodule foreach submodule-update.sh "$PWD/.gitmodules"
    echo "========================================="
    echo "Exiting recursive scan of the $reponame's submodules!"
    echo "========================================="
fi
