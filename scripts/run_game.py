#!/usr/bin/env python3
from __future__ import annotations

import os
import sys
from typing import NoReturn

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
PY_WRAPPER_BUILD = os.path.join(ROOT, "build_python", "src", "py_wrapper")
PY_WRAPPER_SRC = os.path.join(ROOT, "src", "py_wrapper")

sys.path.append(PY_WRAPPER_BUILD)
sys.path.append(PY_WRAPPER_SRC)

try:
    from ant_core import AntGameFacade
except ImportError as exc:
    msg = str(exc)
    if "Python version mismatch" in msg:
        raise SystemExit(
            "Error: ant_core was built for a different Python version.\n"
            "Try running with python3.10:\n"
            "  python3.10 scripts/run_game.py\n"
            "Or rebuild the extension with your current Python:\n"
            "  cmake -S . -B build_python -DCMAKE_BUILD_TYPE=Release\n"
            "  cmake --build build_python --target ant_core\n"
        ) from exc
    raise


def main() -> NoReturn:
    game = AntGameFacade()
    while True:
        game.update()


if __name__ == "__main__":
    main()
