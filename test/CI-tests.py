#!/usr/bin/python3

#
#  TheXTech - A platform game engine ported from old source code for VB6
#
#  Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
#  Copyright (c) 2020-2024 Vitaly Novichkov <admin@wohlnet.ru>
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

import argparse
import subprocess
import json
import sys
import os

parser = argparse.ArgumentParser(
                    prog='CI-tests.py',
                    description='This script is a WIP way to measure the game\'s compatibility and performance on using gameplay recordings.')

parser.add_argument('-n', '--name', help='name to record this test with', required=True)
parser.add_argument('-e', '--executable', help='path to the game executable (should generally be a command-line build)', required=True)
parser.add_argument('-d', '--records-dir', help='directory of gameplay records to use', required=True)
parser.add_argument('-o', '--output', help='output file to append a CSV row to')
args = parser.parse_args()

title = args.name
binary = args.executable
test_dir = args.records_dir

benches = [os.path.join(test_dir, test) for test in os.listdir(test_dir) if test.endswith('.rec')]

sections = subprocess.check_output(['size', binary]).decode().strip().split('\n')[1].split()
text = int(sections[0]) // 1024
static_ram = (int(sections[1]) + int(sections[2])) // 1024

max_heap = 0
total_instructions = 0
total_cycles = 0

total_pass = 0
total_warn = 0
total_fail = 0
total_invalid = 0

print(f'Code size {text} KB, static RAM use {static_ram} KB')

for bench in benches:
    print(f'On {bench}...')
    output_lines = subprocess.check_output(['/usr/bin/time', '-f', '%M', 'perf', 'stat', '-j', binary, bench], stderr=subprocess.STDOUT).decode().strip().split('\n')

    # 1-2 lines to indicate logs, then results
    if output_lines[1].strip() == '':
        start = 2
    else:
        start = 1

    test_result = output_lines[start]
    mem_peak = int(output_lines[-1])
    stack_heap = mem_peak - (text + static_ram)

    if stack_heap > max_heap:
        max_heap = stack_heap


    for line in output_lines[start + 1:-1]:
        line_out = json.loads(line)

        if not 'event' in line_out:
            continue

        if line_out['event'] == 'cycles:u':
            cycles = int(float(line_out['counter-value']))
        elif line_out['event'] == 'instructions:u':
            instructions = int(float(line_out['counter-value']))

    total_instructions += instructions
    total_cycles += cycles

    if 'CONGRATULATIONS' in test_result:
        total_pass += 1
    elif 'MINOR' in test_result:
        total_warn += 1
    elif 'DIVERGED' in test_result:
        total_fail += 1
    else:
        total_invalid += 1

    print(f'  {test_result}')
    print(f'  Used {stack_heap} KB RAM')
    print(f'  Took {instructions} instructions ({cycles} cycles)')

# an output to be added to a CSV
template = 'title,pass,warn,fail,invalid,codesizeKB,staticmemKB,maxheapKB,Minstructions,Mcycles'
output_row = f'{title},{total_pass},{total_warn},{total_fail},{total_invalid},{text},{static_ram},{max_heap},{total_instructions // 1000000},{total_cycles // 1000000}'

if args.output:
    open(args.output, 'a').write(output_row+'\n')
else:
    print()
    print('    The template is:')
    print(template)
    print('    The next row should be:')
    print(output_row)
