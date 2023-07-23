#! /usr/bin/env python3
"""
This script adds a license file to a DMG. Requires Xcode and a plain ascii text
license file.
Obviously only runs on a Mac.

Copyright (C) 2011 Jared Hobbs

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
"""
import os
import sys
import tempfile
import optparse


class Path(str):
    def __enter__(self):
        return self

    def __exit__(self, in_type, value, traceback):
        os.unlink(self)


def mktemp(dirname=None, suffix=''):
    (fd, filename) = tempfile.mkstemp(dir=dirname, suffix=suffix)
    os.close(fd)
    return Path(filename)


def main(in_options, in_args):
    dmg_file, license_file = in_args
    with mktemp('.') as tmpFile:
        with open(tmpFile, 'w') as f:
            f.write("""data 'LPic' (5000) {
    $"0002 0011 0003 0001 0000 0000 0002 0000"
    $"0000 000E 0006 0001 0005 0007 0000 0007"
    $"0008 0000 0047 0009 0000 0034 000A 0001"
    $"0035 000B 0001 0020 000C 0000 0011 000D"
    $"0000 005B 0004 0000 0033 000F 0001 000C"
    $"0010 0000 000B 000E 0000"
};\n\n""")
            with open(license_file, 'r') as l:
                f.write('data \'TEXT\' (5002, "English") {\n')
                for line in l:
                    if len(line) < 1000:
                        f.write('    "' + line.strip().replace('"', '\\"') +
                                '\\n"\n')
                    else:
                        for liner in line.split('.'):
                            f.write('    "' +
                                    liner.strip().replace('"', '\\"') +
                                    '. \\n"\n')
                f.write('};\n\n')
            f.write("""resource 'STR#' (5002, "English") {
    {
        "English",
        "Agree",
        "Disagree",
        "Print",
        "Save...",
        "IMPORTANT - By clicking on the \\"Agree\\" button, you agree "
        "to be bound by the terms of the License Agreement.",
        "Software License Agreement",
        "This text cannot be saved. This disk may be full or locked, or the "
        "file may be locked.",
        "Unable to print. Make sure you have selected a printer."
    }
};""")
        os.system('/usr/bin/hdiutil unflatten -quiet "%s"' % dmg_file)
        os.system('%s "%s/"*.r %s -a -o "%s"' %
                  (in_options.rez, in_options.flat_carbon, tmpFile, dmg_file))

        os.system('/usr/bin/hdiutil flatten -quiet "%s"' % dmg_file)
        if in_options.compression is not None:
            os.system('cp %s %s.temp.dmg' % (dmg_file, dmg_file))
            os.remove(dmg_file)
            if in_options.compression == "bz2":
                os.system('hdiutil convert %s.temp.dmg -format UDBZ -o %s' %
                          (dmg_file, dmg_file))
            elif in_options.compression == "gz":
                os.system('hdiutil convert %s.temp.dmg -format ' % dmg_file +
                          'UDZO -imagekey zlib-devel=9 -o %s' % dmg_file)
            os.remove('%s.temp.dmg' % dmg_file)
    print("Successfully added license to '%s'" % dmg_file)


if __name__ == '__main__':
    parser = optparse.OptionParser()
    parser.set_usage("""%prog <dmgFile> <licenseFile> [OPTIONS]
  This program adds a software license agreement to a DMG file.
  It requires Xcode and a plain ascii text <licenseFile>.

  See --help for more details.""")
    parser.add_option(
        '--rez',
        '-r',
        action='store',
        default='/Applications/Xcode.app/Contents/Developer/Tools/Rez',
        help='The path to the Rez tool. Defaults to %default'
    )
    parser.add_option(
        '--flat-carbon',
        '-f',
        action='store',
        default='/Applications/Xcode.app/Contents/Developer/Platforms'
                '/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk'
                '/Developer/Headers/FlatCarbon',
        help='The path to the FlatCarbon headers. Defaults to %default'
    )
    parser.add_option(
        '--compression',
        '-c',
        action='store',
        choices=['bz2', 'gz'],
        default=None,
        help='Optionally compress dmg using specified compression type. '
             'Choices are bz2 and gz.'
    )
    options, args = parser.parse_args()
    cond = len(args) != 2 or not os.path.exists(options.rez) \
        or not os.path.exists(options.flat_carbon)
    if cond:
        parser.print_usage()
        sys.exit(1)
    main(options, args)
