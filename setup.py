from __future__ import annotations

from pathlib import Path

from setuptools import Extension, find_packages, setup

ROOT = Path(__file__).parent.resolve()

core_sources = [
    "src/BLEU.cpp",
    "src/datareader.cpp",
    "src/heuristic.cpp",
    "src/knapsack.cpp",
    "src/master_milp.cpp",
    "src/preprocessing.cpp",
    "src/skyline.cpp",
    "src/spp.cpp",
]

ext_modules = [
    Extension(
        "spp._spp_native",
        sources=["python/spp/_spp_native.cpp", *core_sources],
        include_dirs=["src", "third_party/highs/include", "third_party/highs/include/highs"],
        library_dirs=["third_party/highs/lib"],
        libraries=["highs", "z", "m", "pthread"],
        language="c++",
        extra_compile_args=["-std=c++17"],
    )
]

setup(
    packages=find_packages(where="python"),
    package_dir={"": "python"},
    package_data={"spp": ["py.typed", "__init__.pyi"]},
    ext_modules=ext_modules,
)
