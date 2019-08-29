import os
import sys
sys.path.append('..')
sys.path.append('../..')
from chowdren.common import call, makedirs
from chowdren.converter import Converter

CMAKE_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__),
                                          'CMake', 'bin', 'cmake.exe'))

DISTRO_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), 'MinGW'))
MAKE_PATH = os.path.join(DISTRO_PATH, 'bin', 'make.exe')
LIB_PATH = os.path.join(DISTRO_PATH, 'lib')

BUILD_TYPES = {
    0: 'win',
    1: 'winsrc',
    2: 'src'
}

class Arguments(object):
    def __init__(self, args):
        self.__dict__ = args

class Builder(object):
    def __init__(self, build_type, src, target):
        self.build_type = BUILD_TYPES[build_type]
        self.src = src

        if self.build_type == 'win':

        self.target = os.path.abspath(src_dir)
        self.build_dir = os.path.join(self.src_dir, 'build')

    def run(self):
        self.convert()
        self.set_environment()
        self.create_project()
        self.build_project()
        if self.build_type == 'src':
            return
        self.copy_project()

    def convert(self):
        args = {
            'outdir': self.src_dir,
            'dlls': False,
            'filenames': [self.src],
            'platform': 'generic',
            'config': None,
            'skipassets': False,
            'ico': None,
            'icns': None,
            'copyright': None,
            'company': None,
            'author': None,
            'version': None
        }

        Converter(Arguments(args))

    def create_project(self):
        cwd = os.getcwd()
        makedirs(self.build_dir)
        os.chdir(self.build_dir)
        call([CMAKE_PATH, '..', '-G', 'MinGW Makefiles',
              '-DCMAKE_LIBRARY_PATH:PATH=%s' % LIB_PATH])
        os.chdir(cwd)

    def build_project(self):
        cwd = os.getcwd()
        os.chdir(self.build_dir)
        call([MAKE_PATH, '-j4'])
        os.chdir(cwd)

    def run_project(self):
        cwd = os.getcwd()
        os.chdir(self.src_dir)
        exe = os.path.join(self.build_dir, 'Chowdren.exe')
        call([exe])
        os.chdir(cwd)

    def copy_exec

    def set_environment(self):
        os.environ['X_DISTRO'] = 'nuwen'
        includes = (os.path.join(DISTRO_PATH, 'include'),
                    os.path.join(DISTRO_PATH, 'include', 'freetype2'))
        includes = ';'.join(includes)
        path = os.environ['PATH'].split(';')
        path = [os.path.join(DISTRO_PATH, 'bin'),
                os.path.join(DISTRO_PATH, 'git', 'cmd')] + path
        os.environ['PATH'] = ';'.join(path)
        os.environ['C_INCLUDE_PATH'] = includes
        os.environ['CPLUS_INCLUDE_PATH'] = includes
        os.environ['SDL2DIR'] = LIB_PATH

def main():
    builder = Builder(int(sys.argv[1]), sys.argv[2], sys.argv[3])
    builder.run()

if __name__ == '__main__':
    main()