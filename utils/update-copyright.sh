#!/bin/bash

find . -type f -exec grep -Il "Copyright" {} \;     \
| grep -v \.git \
| while read file;                            \
do \
  LC_ALL=C sed -b -i "s/\(.*Copyright.*\)[0-9]\{4\}\( *Vitaly Novichkov\)/\1`date +%Y`\2/" "$file"; \
done

if [[ -f "pge_version.h" ]]; then
    LC_ALL=C sed -b -i "s/\(.*V_COPYRIGHT.*\)[0-9]\{4\}\( *by Wohlstand\)/\1`date +%Y`\2/" "pge_version.h";
fi
