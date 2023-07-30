#!/usr/bin/python3

# this should be fixed later but currently delegates to the other two scripts in the directory

import os
import sys
import subprocess
import argparse

parser = argparse.ArgumentParser(
    description='Converts TheXTech assets for use on Nintendo homebrew devices')

parser.add_argument('input')
parser.add_argument('output')
parser.add_argument('-t', '--target', choices=['3ds', 'wii'], required=True)

args = parser.parse_args()

script_directory = os.path.dirname(os.path.abspath(sys.argv[0]))

if args.target == '3ds':
    subprocess.run(['python3', os.path.join(script_directory, 'gfx-convert-3ds.py'), args.input, args.output])
elif args.target == 'wii':
    subprocess.run(['python3', os.path.join(script_directory, 'gfx-convert-wii.py'), args.input, args.output])
