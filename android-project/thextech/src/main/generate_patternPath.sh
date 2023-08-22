#!/bin/bash

for q in lvl LVL wld WLD lvlx LVLX wldx WLDX; do
    echo "                <data android:pathPattern=\".*\\\\.${q}\" />"
    echo "                <data android:pathPattern=\".*\\\\..*\\\\.${q}\" />"
    echo "                <data android:pathPattern=\".*\\\\..*\\\\..*\\\\.${q}\" />"
    echo "                <data android:pathPattern=\".*\\\\..*\\\\..*\\\\..*\\\\.${q}\" />"
    echo "                <data android:pathPattern=\".*\\\\..*\\\\..*\\\\..*\\\\..*\\\\.${q}\" />"
    echo "                <data android:pathPattern=\".*\\\\..*\\\\..*\\\\..*\\\\..*\\\\..*\\\\.${q}\" />"
    echo "                <data android:pathPattern=\".*\\\\..*\\\\..*\\\\..*\\\\..*\\\\..*\\\\..*\\\\.${q}\" />"
    echo ""
done
