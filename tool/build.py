import os
import sys
import subprocess

def extract_raylib():
    if not os.path.exists("build"):
        os.mkdir("build")
    if sys.platform == "win32":
        if os.path.exists("build/raylib-5.5_win64_mingw-w64"):
            return
        os.chdir("build")
        subprocess.run([
            "powershell", "-Command",
            "Expand-Archive", "../bin/raylib-5.5_win64_mingw-w64.zip",
            "-DestinationPath", "."
        ])
        os.chdir("..")
    elif sys.platform == "darwin":
        if os.path.exists("build/raylib-5.5_macos"):
            return
        os.chdir("build")
        subprocess.run([
            "tar", "-xvzf", "../bin/raylib-5.5_macos.tar.gz"
        ])
        os.chdir("..")

def build_cmake():
    if not os.path.exists("build_cmake"):
        os.mkdir("build_cmake")
    os.chdir("build_cmake")
    if sys.platform == "win32":
        subprocess.run([
            "cmake", "-G", "MinGW Makefiles", "..",
            "-DCMAKE_BUILD_TYPE=RelWithDebInfo"
        ]).check_returncode()
    else:
        subprocess.run([
            "cmake", "..",
            "-DCMAKE_BUILD_TYPE=RelWithDebInfo"
        ]).check_returncode()
    if sys.platform == "win32":
        subprocess.run([
            "mingw32-make.exe", "-j4"
        ]).check_returncode()
    else:
        subprocess.run([
            "make", "-j4"
        ]).check_returncode()
    os.chdir("..")

if __name__ == "__main__":
    extract_raylib()
    build_cmake()