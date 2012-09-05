import sys
sys.path.append('..')

from chowdren.converter import Converter
import argparse

def main():
    parser = argparse.ArgumentParser(description='Chowdren')
    parser.add_argument('filename', type = str, help = 'input file to convert')
    parser.add_argument('outdir', type = str, help = 'destination directory')
    args = parser.parse_args()
    
    Converter(args.filename, args.outdir)
    
if __name__ == '__main__':
    main()