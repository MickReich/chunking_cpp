from setuptools import setup, find_packages

setup(
    name="chunking_cpp",
    version="0.1",
    packages=find_packages(where="build/python"),
    package_dir={"": "build/python"},
    package_data={"chunking_cpp": ["*.so"]},
) 