import os
import sys
import subprocess

if not os.path.exists("build"):
    os.mkdir("build")
if sys.platform == "win32":
    if not os.path.exists("build/raylib-5.5_win64_mingw-w64"):
        os.chdir("build")
        subprocess.run([
            "powershell", "-Command",
            "Expand-Archive", "../bin/raylib-5.5_win64_mingw-w64.zip",
            "-DestinationPath", "."
        ])
        os.chdir("..")
elif sys.platform == "darwin":
    if not os.path.exists("build/raylib-5.5_macos"):
        os.chdir("build")
        subprocess.run([
            "tar", "-xvzf", "../bin/raylib-5.5_macos.tar.gz"
        ])
        os.chdir("..")

if not os.path.exists("build_cmake"):
    os.mkdir("build_cmake")

if __name__ == "__main__":
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