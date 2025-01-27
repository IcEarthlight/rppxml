from setuptools import setup, Extension
import pybind11

ext_modules = [
    Extension(
        "rppxml",
        ["src/rppxml.cpp"],
        include_dirs = [pybind11.get_include()],
        language = "c++"
    ),
]

setup(
    name = "rppxml",
    version = "0.1",
    author = "IcEarthlight",
    author_email = "earthlight2187@hotmail.com",
    description = "RPP XML parser using WDL implementation",
    ext_modules = ext_modules,
    package_data = {
        "rppxml": ["*.pyi"],  # include type stub files
    },
    zip_safe = False,  # required for mypy to find .pyi files
    python_requires = ">=3.10",  # updated to 3.10+ for type union operator support
) 
