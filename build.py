import os, subprocess, sys, shutil

class tdbuild():
    def __init__(self):
        self.source_dir = os.path.join("..", "src")
        self.include_dirs = [
            os.path.join("..", "include"),
            os.path.join("..", "assets"),
            os.path.join("..", "assets", "shaders")
        ]
        self.lib_dir = os.path.join("..", "lib")
        self.build_dir = "build"
        
        self.system_libs = ["user32.lib", "opengl32.lib","gdi32.lib","Shell32.lib"]
        self.user_libs = ["glfw3.lib", "freetyped.lib"]
        
        self.source_files = ["main.c", "glad.c"]
        self.ignore = ["4099"]
        self.out = "tdeditor.exe"
        self.runtime_library = "MTd"
        self.debug = True
        self.cpp = False
        
        self.build_cmd = ""

    def push(self, item):
        self.build_cmd = self.build_cmd + item + " "

    def build(self):
        print("Building from", os.getcwd())
        
        self.push("cl.exe")

        if self.cpp:
            self.push("/TP")
        else:
            self.push("/TC")

        if self.debug :
            self.push("-Zi")

        self.push("-" + self.runtime_library)

        for source_file in self.source_files:
            full_source_file_path = os.path.join(self.source_dir, source_file)
            self.push(full_source_file_path)

        for include_dir in self.include_dirs:
            self.push('/I\"{}\"'.format(include_dir))

        self.push("/link")
        for system_lib in self.system_libs:
            self.push(system_lib)

        for user_lib in self.user_libs:
            self.push(os.path.join(self.lib_dir, user_lib))
            
        for ignore in self.ignore:
            self.push("/ignore:" + ignore)

        self.push("/out:" + self.out)

        build_dir = os.path.join(os.getcwd(), self.build_dir)
        try:
            os.mkdir(build_dir)
        except:
            pass
        os.chdir(build_dir)
        
        # @hack: is there a better way to keep a process open?
        print(self.build_cmd)
        subprocess.run("{} && {}".format(\
            os.path.join("..", "setup_devenv.bat"),
            self.build_cmd))

        os.chdir("..")
        
    def run(self):
        os.chdir(os.path.dirname(os.path.realpath(__file__)))
        subprocess.run([os.path.join(os.getcwd(), self.build_dir, self.out)])
                 

if __name__ == "__main__":
    builder = tdbuild()
    
    if not len(sys.argv) or sys.argv[1] == "build":
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

        
        
    
