#!/bin/bash
echo "$1 ($2 -> $3)"
sed -i "s/PlaySound($2)/PlaySound($3)/g" $1
