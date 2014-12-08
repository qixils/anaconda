import os
import sys
sys.path.append('..')
sys.path.append('../..')
from chowdren.common import call, makedirs

CMAKE_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__),
                                          'CMake', 'bin', 'cmake.exe'))

DISTRO_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), 'MinGW'))
MAKE_PATH = os.path.join(DISTRO_PATH, 'bin', 'make.exe')
LIB_PATH = os.path.join(DISTRO_PATH, 'lib')

class Builder(object):
    def __init__(self, src, src_dir):
        self.src = src
        self.src_dir = src_dir
        self.build_dir = os.path.join(src_dir, 'build')

    def run(self):
        self.set_environment()
        self.create_project()
        self.build_project()

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
    builder = Builder(sys.argv[1], sys.argv[2])
    builder.run()

if __name__ == '__main__':
    main()