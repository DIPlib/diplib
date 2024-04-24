#!/usr/bin/env python3

import argparse
import re
import sys

import diplib as dip


def main(args=None):
    parser = argparse.ArgumentParser(prog='dipview',
                                     description='Displays DIPlib-readable image files using DIPviewer')

    parser.add_argument('files', metavar='file', nargs='+', help='image file to display')
    parser.add_argument('-b', '--bioformats', action='store_true',
                        help='forces the use of Bio-Formats for all file types')

    args = parser.parse_args()

    first = None
    sizes = None
    for f in args.files:
        if args.bioformats:
            image = dip.ImageRead(f, 'bioformats')
        else:
            image = dip.ImageRead(f)

        handle = dip.viewer.Show(image)
        if first is None:
            first = handle
            sizes = image.Sizes()
        elif image.Sizes() == sizes:
            handle.Link(first)

    dip.viewer.Spin()
    dip.viewer.CloseAll()


if __name__ == '__main__':
    sys.argv[0] = re.sub(r'(-script\.pyw|\.exe)?$', '', sys.argv[0])
    sys.exit(main())
