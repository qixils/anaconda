import sys
sys.path.append('..')

from chowdren.converter import Converter
import argparse

def main():
    parser = argparse.ArgumentParser(
        description = 'Chowdren - MMF to C++ converter')
    parser.add_argument('filename', type = str, 
        help = 'input file to convert (should be an EXE or CCN file)')
    parser.add_argument('outdir', type = str, help = 'destination directory')
    parser.add_argument('--imagedir', type = str, action = 'store',
        default = 'images', help = 'destination directory for images')
    parser.add_argument('--noimages', action='store_true',
        help = 'turns off image writing')
    args = parser.parse_args()
    if args.noimages:
        image_dir = None
    else:
        image_dir = args.imagedir
    Converter(args.filename, args.outdir, image_dir)
    
if __name__ == '__main__':
    main()