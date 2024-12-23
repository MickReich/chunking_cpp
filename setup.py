from setuptools import setup, find_packages
import os

# Ensure the package directory exists
package_dir = os.path.join("build", "python")
if not os.path.exists(package_dir):
    os.makedirs(package_dir)

setup(
    name=str("chunking_cpp"),
    version="0.1",
    packages=find_packages(where="build/python"),
    package_dir={"": "build/python"},
    package_data={"chunking_cpp": ["*.so", "*.pyd"]},
    include_package_data=True,
    description="C++ chunking library with Python bindings",
    python_requires=">=3.6",
) 