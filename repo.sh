#!/bin/bash

bak=~+

function checkState()
{
    if [[ $? -eq 0 ]]
    then
        printf "=== \E[37;42mOK!\E[0m ===\n\n"
    else
        printf "\n=== AN ERROR OCCURRED! ===\n\n"
        cd ${bak}
        return 1
    fi
}

flag_pack_src=false
flag_pack_src_gz=false
flag_pack_src_zip=false

for var in "$@"
do
    case "$var" in
        lupdate)
            #dummy
        ;;
        lrelease)
            #dummy
        ;;
        --help)
            echo ""
            printf "=== \e[44mRepo maintenance helper script for TheXTech\e[0m ===\n"
            echo ""
            echo "--- Actions ---"
            printf " \E[1;4mupdate-submodules\E[0m- Pull all submodules up to their latest states\n"
            printf " \E[1;4mrepair-submodules\E[0m- Repair invalid or broken submodules\n"
            printf " \E[1;4m--help\E[0m           - Print this manual\n"
            printf " \E[1;4mpack-src\E[0m         - Create the source code archive\n"
            printf "                    (git-archive-all is required!)\n"
            if [[ "$(which git-archive-all)" == "" ]]; then
                printf " \E[0;4;41;37m<git-archive-all is not installed!>\E[0m\n"
            fi
            echo ""
            exit 1
            ;;

        update-submodules)
            git submodule foreach git checkout master
            git submodule foreach git pull origin master
            cd 3rdparty/LuaJIT
            git checkout v2.1
            git pull origin v2.1
            cd ../..
            exit 0
            ;;

        repair-submodules)
            echo "=== Cleaning-up old state..."
            git submodule foreach 'pwd; rm -Rf * .git*;'
            echo "=== Fetching new submodules..."
            git submodule init
            git submodule update
            echo ""
            git submodule foreach git checkout master
            git submodule foreach git pull origin master
            cd 3rdparty/LuaJIT
            git checkout v2.1
            git pull origin v2.1
            cd ../..
            echo ""
            echo "==== Fixed! ===="
            exit 0;
            ;;

        pack-src)
            flag_pack_src=true
            ;;
        gz)
            flag_pack_src_gz=true
            ;;
        zip)
            flag_pack_src_zip=true
            ;;

        *)
            echo "--------------------------------------------------------------------"
            echo "Invalid argument '$var', please type '$0 --help' to get the usage."
            echo "--------------------------------------------------------------------"
            exit 1;
        ;;
    esac
done

# ===== Source code packer =====
if ${flag_pack_src} ; then
    if [[ ! -d build-archives ]]; then
        mkdir build-archives
    fi
    if ${flag_pack_src_gz} ; then
        ARFORMAT=tar.gz
    elif ${flag_pack_src_zip} ; then
        ARFORMAT=zip
    else
        ARFORMAT=tar.bz2
    fi

    echo "Packing source code..."
    git archive-all -v --force-submodules build-archives/thextech-full-src.${ARFORMAT}
    checkState

    printf "\n=== Packed! ===\n\n"
    cd ${bak}

    exit 0;
fi
