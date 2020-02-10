#!/bin/bash

function cpM()
{
    cp orig/$1 $2
}

HEADS=""
function cpH()
{
    cp orig/$1 $2
    HEADS="${HEADS} $1"
}


cpM format.cc       fmt_format.cpp
cpM ostream.cc      fmt_ostream.cpp
cpM posix.cc        fmt_posix.cpp
cpM printf.cc       fmt_printf.cpp

cpH container.h     fmt_container.h
cpH format.h        fmt_format.h
cpH ostream.h       fmt_ostream.h
cpH posix.h         fmt_posix.h
cpH printf.h        fmt_printf.h
cpH string.h        fmt_string.h
cpH time.h          fmt_time.h

for h in $HEADS; do
    for f in *.cpp; do
        sed -i 's/\"'$h'\"/\"fmt_'$h'\"/g' $f
    done
    for f in *.h; do
        sed -i 's/\"'$h'\"/\"fmt_'$h'\"/g' $f
    done
done

