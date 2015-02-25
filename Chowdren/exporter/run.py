import os
import sys
sys.path.append('..')
sys.path.append('../..')
from chowdren.common import makedirs
from chowdren.converter import Converter
import tempfile
import shutil
import subprocess

CMAKE_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__),
                                          'CMake', 'bin', 'cmake.exe'))

DISTRO_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), 'MinGW'))
MAKE_PATH = os.path.join(DISTRO_PATH, 'bin', 'mingw32-make.exe')
LIB_PATH = os.path.join(DISTRO_PATH, 'lib')

class StopBuild(Exception):
    pass

def call(args):
    exp = ' '.join(args)
    print exp
    print ''
    p = subprocess.Popen(args,
                         stdout=subprocess.PIPE,
                         stderr=subprocess.STDOUT)

    while True:
        for line in iter(p.stdout.readline, ''):
            sys.stdout.write(line)
        retcode = p.poll()
        if retcode is None:
            continue
        if retcode:
            raise StopBuild()
        break

class Arguments(object):
    def __init__(self, args):
        self.__dict__ = args

class Builder(object):
    def __init__(self, platform, src, src_dir=None, executable=None):
        self.platform = platform
        self.src = src

        self.is_temp = src_dir is None
        if self.is_temp:
            src_dir = tempfile.mkdtemp()
        self.src_dir = os.path.abspath(src_dir)

        self.executable = None
        if executable is not None:
            self.executable = os.path.abspath(executable)

        self.build_dir = os.path.join(self.src_dir, 'build')

    def run(self):
        self.convert()
        self.set_environment()
        self.create_project()
        self.build_project()
        self.clean_project()

    def convert(self):
        args = {
            'outdir': self.src_dir,
            'dlls': False,
            'filenames': [self.src],
            'platform': self.platform,
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
        args = [CMAKE_PATH, '..', '-G', 'MinGW Makefiles',
                '-DCMAKE_LIBRARY_PATH:PATH=%s' % LIB_PATH,
                '-DCMAKE_EXE_LINKER_FLAGS:STRING=-static-libgcc '
                '-static-libstdc++ -static']

        if self.executable is not None:
            self.executable_dir = os.path.dirname(self.executable)
            args += ['-DCMAKE_INSTALL_PREFIX:PATH=%s' % self.executable_dir]

        call(args)

        os.chdir(cwd)

    def build_project(self):
        if self.executable is None:
            return
        cwd = os.getcwd()
        os.chdir(self.build_dir)
        call([MAKE_PATH, '-j4'])
        call([MAKE_PATH, 'install'])

        src = os.path.join(self.executable_dir, 'Chowdren.exe')
        os.rename(src, self.executable)
        shutil.copy(os.path.join(self.src_dir, 'Assets.dat'),
                    self.executable_dir)
        os.chdir(cwd)

    def clean_project(self):
        if not self.is_temp:
            return
        shutil.rmtree(self.src_dir, True)

    def run_project(self):
        cwd = os.getcwd()
        os.chdir(self.src_dir)
        exe = os.path.join(self.build_dir, 'Chowdren.exe')
        call([exe])
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
    build_type = int(sys.argv[1])
    src = sys.argv[2]

    executable = src_dir = None
    if build_type == 0:
        executable = sys.argv[3]
    elif build_type == 1:
        src_dir = sys.argv[3]
        executable = sys.argv[4]
    else:
        src_dir = sys.argv[3]

    builder = Builder('generic', src, src_dir, executable)
    builder.run()

if __name__ == '__main__':
    main()