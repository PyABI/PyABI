#!/usr/bin/env python3
# encoding: utf-8

import setuptools

with open("README.md", "r") as fh:
    long_description = fh.read()

setuptools.setup(
    name="PyABI",
    version="0.0.42",
    author="Scott McCallum (github.com scott91e1)",
    author_email="262464@195387.com",
    description="Core Implimation of the C++ PyABI Specification PEP-TBA",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/PyABI/PyABI",
    packages=setuptools.find_packages(),
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],
    python_requires='>=3.6',
)
