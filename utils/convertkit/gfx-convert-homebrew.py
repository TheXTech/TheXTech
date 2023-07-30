#!/usr/bin/python3

# this should be fixed later but currently delegates to the other two scripts in the directory

import os
import argparse

parser = argparse.ArgumentParser(
    description='Converts TheXTech assets for use on Nintendo homebrew devices')

parser.add_argument('input')
parser.add_argument('output')
parser.add_argument('-t', '--target', choices=['3ds',' wii'])

args = parser.parse_args()

if args.target == '3ds':
print(args.filename, args.count, args.verbose)

datadir = sys.argv[1]
graphicsdir = os.path.join(datadir, 'graphics')
outdir = sys.argv[2]
