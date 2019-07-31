import os, subprocess, sys, shutil, platform, colorama

colorama.init()

build_options = {
    'source_dir': os.path.join("..", "src"),
    'include_dirs': [
        os.path.join("..", "include"),
        os.path.join("..", "assets"),
        os.path.join("..", "assets", "shaders")
    ],
    'lib_dir': os.path.join("..", "lib"),
    'build_dir': "build",
    'source_files': [
        "main.c",
        "glad.c"
    ],
    'debug': True,
    'cpp': False,
    'Windows': {
        'system_libs': ["user32.lib", "opengl32.lib","gdi32.lib","Shell32.lib"],
        'user_libs': ["glfw3.lib", "freetyped.lib"],
        'ignore': ["4099"],
        'out': "tdeditor.exe",
        'runtime_library': "MTd",
    },
    'Darwin': {
        'compiler': 'gcc',
        'user_libs': ['libfreetype.a', 'libglfw3.a'],
        'system_libs': ['bz2', 'z'],
        'frameworks': ['Cocoa', 'OpenGL', 'CoreVideo', 'IOKit'],
        'out': 'tdeditor',
        'switches': [
            'Wall',
            'Wno-char-subscripts',
            'Wno-unused-variable',
            'Wno-missing-braces',
            'Wno-unused-result'
        ]
    }
}

def make_cd_build_dir():
    build_dir = os.path.join(os.getcwd(), build_options['build_dir'])
    try:
        os.mkdir(build_dir)
    except:
        pass
    os.chdir(build_dir)

class tdbuild():
    def __init__(self):        
        self.build_cmd = ""
        self.unix_args = []

    def push(self, item):
        self.build_cmd = self.build_cmd + item + " "
        

    def build(self):
        if platform.system() == 'Windows':
            self.build_windows()
        elif platform.system() == 'Darwin':
            self.build_mac()
        
    def build_mac(self):
        print("...building from", os.getcwd())

        # Find the path to the compiler using 'which'
        compiler = build_options['Darwin']['compiler']
        process = subprocess.Popen(['which', compiler], stdout=subprocess.PIPE)
        compiler_path, err = process.communicate()
        compiler_path = compiler_path.decode('UTF-8').strip()
        if err:
            print(colorama.Fore.RED + '[ERROR]')
            print(colorama.Fore.RED + "which {} errored out, so not sure what's up with that".format(compiler))
            
        self.push(compiler_path)
        
        if build_options['debug']:
            self.push("-g")

        for switch in build_options['Darwin']['switches']:
            self.push("-" + switch)

        for source in build_options['source_files']:
            self.push(os.path.join(build_options['source_dir'], source))

        for include in build_options['include_dirs']:
            self.push("-I" + include)

        for lib in build_options['Darwin']['user_libs']:
            self.push(os.path.join(build_options['lib_dir'], lib))

        for lib in build_options['Darwin']['system_libs']:
            self.push('-l' + lib)

        for framework in build_options['Darwin']['frameworks']:
            self.push('-framework ' + framework)

        self.push('-o ' + build_options['Darwin']['out'])
        
        print("...generated compiler command:")
        print(self.build_cmd)
        print("...building")

        # Run the command you just generated. Use os.system() because
        # subprocess.run() wants a list but doesn't play nice with it!
        make_cd_build_dir()
        os.system(self.build_cmd)

        print(colorama.Fore.GREEN + "[BUILD SUCCESSFUL]")
        
    def build_windows(self):
        print(colorama.Fore.BLUE + "[tdbuild] " + colorama.Fore.RESET + "Running from {}".format(os.getcwd()))
        
        self.push("cl.exe")

        self.push("/nologo")

        if build_options['cpp']:
            self.push("/TP")
        else:
            self.push("/TC")

        if build_options['debug']:
            self.push("-Zi")

        self.push("-" + build_options['Windows']['runtime_library'])

        for source_file in build_options['source_files']:
            full_source_file_path = os.path.join(build_options['source_dir'], source_file)
            self.push(full_source_file_path)

        for include_dir in build_options['include_dirs']:
            self.push('/I\"{}\"'.format(include_dir))

        self.push("/link")
        for system_lib in build_options['Windows']['system_libs']:
            self.push(system_lib)

        for user_lib in build_options['Windows']['user_libs']:
            self.push(os.path.join(build_options['lib_dir'], user_lib))
            
        for ignore in build_options['Windows']['ignore']:
            self.push("/ignore:" + ignore)

        self.push("/out:" + build_options['Windows']['out'])


        make_cd_build_dir()
        
        print(colorama.Fore.BLUE + "[tdbuild] " + colorama.Fore.RESET + "Generated compiler command:")
        print(colorama.Fore.BLUE + "[tdbuild] " + colorama.Fore.RESET + self.build_cmd)
        print(colorama.Fore.BLUE + "[tdbuild] " + colorama.Fore.RESET + "Invoking the compiler")
        print("")
        
        # @hack: is there a better way to keep a process open?
        process = subprocess.Popen("{} && {}".format(os.path.join("..", "setup_devenv.bat"), self.build_cmd), stdout=subprocess.PIPE)
        compiler_messages, err = process.communicate()
        compiler_messages = compiler_messages.decode('UTF-8').split('\n')
        
        compile_error = False
        compile_warning = False
        for message in compiler_messages:
            if 'error' in message:
                print(colorama.Fore.RED + "[tdbuild] " + colorama.Fore.RESET + message)
                compile_error = True
            elif 'warning' in message:
                print(colorama.Fore.YELLOW + "[tdbuild] " + colorama.Fore.RESET + message)
                compile_warning = True

        os.chdir("..")

        if compile_error or compile_warning:
            print("")
            
        if compile_error:
            print(colorama.Fore.RED + "[BUILD FAILED]")
        else:
            print(colorama.Fore.GREEN + "[BUILD SUCCESSFUL]")

        
    def run(self):
        os.chdir(os.path.dirname(os.path.realpath(__file__)))
        subprocess.run([os.path.join(os.getcwd(), build_options['build_dir'], build_options[platform.system()]['out'])]) 
                 

if __name__ == "__main__":
    builder = tdbuild()
    
    if len(sys.argv) is 1 or sys.argv[1] == "build":
        builder.build()
    elif sys.argv[1] == "run":
        builder.run()
    # Some hacky bullshit to get VS to open my executable in the right directory
    elif sys.argv[1] == "debug":
        # Make sure that this script runs from the project root
        os.chdir(os.path.dirname(os.path.realpath(__file__)))

        copied_files = []
        for fname in os.listdir(builder.build_dir):
            fname = os.path.join(os.getcwd(), builder.build_dir, fname)
            if not os.path.isdir(fname):
                shutil.copy2(fname, ".")
                copied_files.append(fname)
                
        subprocess.call(["devenv.exe", builder.out])

        for fname in copied_files:
            os.remove(fname)

        
        
    
