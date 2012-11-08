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
    parser.add_argument('--ico', type = str, action = 'store', default = None,
        help = 'icon to use for Windows')
    parser.add_argument('--version', type = str, action = 'store', 
        default = None, help = 'version to set in executable')
    parser.add_argument('--company', type = str, action = 'store', 
        default = None, help = 'company to set in executable')
    parser.add_argument('--copyright', type = str, action = 'store', 
        default = None, help = 'copyright to set in executable')
    args = parser.parse_args()
    if args.noimages:
        image_dir = None
    else:
        image_dir = args.imagedir
    Converter(args.filename, args.outdir, images_dir = image_dir, 
              win_ico = args.ico, version = args.version, 
              company = args.company, copyright = args.copyright)
    
if __name__ == '__main__':
    main()