import os

import tdbuild.tdbuild as tdbuild

build_options = {
    'source_dir': os.path.join('..', 'src'),
    'include_dirs': [
        os.path.join('..', 'include')
    ],
    'lib_dir': os.path.join('..', 'lib'),
    'build_dir': 'build',
    'source_files': [
        'main.c',
        'glad.c'
    ],
    'debug': True,
    'cpp': False,
    'Windows': {
        'system_libs': [],
        'user_libs': [],
        'dlls': [],
        'ignore': [],
        'machine': '',
        'out': '',
        'runtime_library': '',
        'warnings': [],
        'extras': []
    },
    'Darwin': {
        'compiler': '',
        'user_libs': [],
        'system_libs': [],
        'frameworks': [],
        'out': '',
        'extras': []
    },
    'Linux': {
        'compiler': 'gcc',
        'user_libs': [
            'freetype',
            'glfw3'
        ],
        'system_libs':[
            'c',
            'X11',
            'dl',
            'm',
            'pthread',
            'GL'
        ],
        'extras': [
            '-Wno-unused-result'
        ],
        'out': 'tdeditor'
    }
}

class Builder(tdbuild.base_builder):
    def __init__(self):
        super().__init__()

    def build(self):
        super().build()
        
    def run(self):
        super().run()
        
    def setup(self):
        here = os.path.dirname(os.path.realpath(__file__))
        config_path = os.path.join(here, 'src', 'tded.conf')
        machine_path = os.path.join(here, 'src', 'machine.h')
        with open(machine_path, 'w') as f:
            f.write(f'static const char* conf = "{config_path}";')
        
    def prebuild(self):
        pass
    
