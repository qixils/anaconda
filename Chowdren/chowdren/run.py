import sys
sys.path.append('..')

from chowdren.converter import Converter
import argparse

def main():
    parser = argparse.ArgumentParser(description='Chowdren')
    parser.add_argument('filename', type = str, help = 'input file to convert')
    parser.add_argument('outdir', type = str, help = 'destination directory')
    parser.add_argument('--imagedir', type = str, 
        help = 'destination directory for images')
    parse.add_argument('--noimages', type = bool, 'turns of image-writing')
    args = parser.parse_args()
    
    Converter(args.filename, args.outdir, getattr(args, 'imagedir', None))
    
if __name__ == '__main__':
    main()