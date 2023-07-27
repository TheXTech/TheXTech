#!/usr/bin/python3

import json
import sys
import optparse


def merge_jsons(my_dict, output):
    for k, v in my_dict.items():
        if isinstance(v, dict):
            if k not in output:
                output[k] = dict()
            merge_jsons(v, output[k])
            continue

        output[k] = v


def main(in_args):
    in_file = in_args[0]
    patch_file = in_args[1]
    out_file = in_args[2]

    with open(in_file, encoding='utf-8') as first_file:
        json_out = json.load(first_file)

    with open(patch_file, encoding='utf-8') as second_file:
        json_in = json.load(second_file)

    print("-- Patching translation file: %s\n"
          "-- By the patch file: %s\n"
          "-- Writing result into the %s" % (in_file, patch_file, out_file))

    merge_jsons(json_in, json_out)

    with open(out_file, 'w', encoding='utf-8') as f:
        json.dump(json_out, f, ensure_ascii=False, indent=4)

    print("Completed!")


if __name__ == '__main__':
    parser = optparse.OptionParser()
    parser.set_usage("""%prog <in file to patch> <patch file> <output file>.

  See --help for more details.""")

    options, args = parser.parse_args()
    cond = len(args) != 3
    if cond:
        parser.print_usage()
        sys.exit(1)

    main(args)
