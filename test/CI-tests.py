#!/usr/bin/python3

#
#  TheXTech - A platform game engine ported from old source code for VB6
#
#  Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
#  Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

# This script is a WIP way to measure the game's compatibility and performance on recordings made with SMBX-R.
# It will be used in the future to automate compatibility and performance testing for TheXTech.

import subprocess
import json

binary = './TheXTech-null'
benches = ['bench/1.rec', 'bench/2.rec', 'bench/7.rec']

sections = subprocess.check_output(['size', binary]).decode().strip().split('\n')[1].split()
text = int(sections[0]) // 1024
static_ram = (int(sections[1]) + int(sections[2])) // 1024

print(f'Code size {text} KB, static RAM use {static_ram} KB')

for bench in benches:
    print(f'On {bench}...')
    output_lines = subprocess.check_output(['/usr/bin/time', '-f', '%M', 'perf', 'stat', '-j', binary, bench], stderr=subprocess.STDOUT).decode().strip().split('\n')

    test_result = output_lines[1]
    mem_peak = int(output_lines[-1])
    stack_heap = mem_peak - (text + static_ram)


    for line in output_lines[2:-1]:
        line_out = json.loads(line)

        if not 'event' in line_out:
            continue

        if line_out['event'] == 'cycles:u':
            cycles = int(float(line_out['counter-value']))
        elif line_out['event'] == 'instructions:u':
            instructions = int(float(line_out['counter-value']))

    print(f'  {test_result}')
    print(f'  Used {stack_heap} KB RAM')
    print(f'  Took {instructions} instructions ({cycles} cycles)')
