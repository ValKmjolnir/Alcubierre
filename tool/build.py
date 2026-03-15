import os
import subprocess

if not os.path.exists("build"):
    os.mkdir("build")
if not os.path.exists("build/raylib-5.5_win64_mingw-w64"):
    os.chdir("build")
    subprocess.run([
        "unzip", "../bin/raylib-5.5_win64_mingw-w64.zip"
    ])
    os.chdir("..")

if not os.path.exists("build_cmake"):
    os.mkdir("build_cmake")

if __name__ == "__main__":
    os.chdir("build_cmake")
    subprocess.run([
        "cmake", "-G", "MinGW Makefiles", "..",
        "-DCMAKE_BUILD_TYPE=RelWithDebInfo"
    ]).check_returncode()
    subprocess.run([
        "make.exe", "-j4"
    ]).check_returncode()
    os.chdir("..")