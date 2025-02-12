import os
import platform
import subprocess
from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext

class CustomBuildExt(build_ext):
    def build_extension(self, ext):
        ext.runtime_library_dirs = []
        super().build_extension(ext)

def init_submodules():
    """Initialize git submodules if they haven't been initialized yet."""
    if not os.path.exists("third_party/WDL/.git"):
        print("Initializing WDL submodule...")
        subprocess.check_call(["git", "submodule", "update", "--init",
                               "--recursive", "third_party/WDL"])
    
    if not os.path.exists("third_party/pybind11/.git"):
        print("Initializing pybind11 submodule...")
        subprocess.check_call(["git", "submodule", "update", "--init",
                               "--recursive", "third_party/pybind11"])

# initialize submodules before building
init_submodules()

# detect the platform
is_windows = platform.system() == "Windows"
is_macos = platform.system() == "Darwin"

# common source files
sources = [
    "src/rppxml.cpp",
    "third_party/WDL/WDL/projectcontext.cpp",
]

# common include directories
include_dirs = [
    "src",
    "third_party/WDL",
    "third_party/pybind11/include",
]

# common compile arguments
extra_compile_args = []
extra_link_args = []

# platform specific settings
if is_windows:
    extra_compile_args.extend(["/std:c++14", "/EHsc"])
else:
    extra_compile_args.extend(["-std=c++14", "-fvisibility=hidden"])
    if is_macos:
        extra_compile_args.extend(["-mmacosx-version-min=10.14"])
        extra_link_args.extend(["-mmacosx-version-min=10.14"])

# define the extension module
ext_modules = [
    Extension(
        "rppxml.rppxml",
        sources = sources,
        include_dirs = include_dirs,
        extra_compile_args = extra_compile_args,
        extra_link_args = extra_link_args,
        language = "c++",
    ),
]

# read README.md for long description
long_description = ""
if os.path.exists("README.md"):
    with open("README.md", "r", encoding="utf-8") as f:
        long_description = f.read()

setup(
    name = "rppxml",
    version = "0.1.3",
    author = "IcEarthlight",
    author_email = "earthlight2187@hotmail.com",
    description = "A REAPER project file (RPP) parser using WDL implementation",
    long_description = long_description,
    long_description_content_type = "text/markdown",
    url = "https://github.com/IcEarthlight/rppxml",
    packages = ["rppxml"],
    package_data = {
        "rppxml": ["rppxml.pyi"],
    },
    ext_modules = ext_modules,
    cmdclass = {'build_ext': CustomBuildExt},
    python_requires = ">=3.10",
    classifiers = [
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
        "Programming Language :: Python :: 3.13",
        "Programming Language :: C++",
        "Topic :: Multimedia :: Sound/Audio",
        "Topic :: Software Development :: Libraries :: Python Modules",
    ],
) 
