"""
Setup script for Thor-san vision system
"""
from setuptools import setup, find_packages

with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

with open("requirements.txt", "r", encoding="utf-8") as fh:
    requirements = [line.strip() for line in fh if line.strip() and not line.startswith("#")]

setup(
    name="thor-san",
    version="0.1.0",
    author="Thor-san Development Team",
    description="Computer vision and spatial intelligence system for 6-DOF robotic arm",
    long_description=long_description,
    long_description_content_type="text/markdown",
    packages=find_packages(),
    classifiers=[
        "Development Status :: 3 - Alpha",
        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",
        "Topic :: Scientific/Engineering :: Artificial Intelligence",
        "Topic :: Scientific/Engineering :: Image Recognition",
        "License :: OSI Approved :: MIT License",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
    ],
    python_requires=">=3.8",
    install_requires=requirements,
    extras_require={
        "dev": [
            "pytest>=7.4.0",
            "black>=23.0.0",
            "flake8>=6.0.0",
            "mypy>=1.0.0",
        ],
        "sam": [
            "segment-anything>=1.0.0",
        ],
    },
    entry_points={
        "console_scripts": [
            "thor-san-test-cameras=experiments.01_test_cameras:main",
            "thor-san-calibrate=experiments.02_calibrate_stereo:main",
            "thor-san-detect=experiments.03_test_detection:main",
            "thor-san-map=experiments.04_build_3d_map:main",
            "thor-san-visualize=experiments.05_visualize_scene:main",
        ],
    },
)
