#!/usr/bin/env python3
from typing import NoReturn
import sys

# add path to ant_core.so
sys.path.append("C:\\Users\\wwodr\\projects\\ants\\out\\build\\x64-Debug\\src\\py_wrapper")
sys.path.append("src/py_wrapper")

from ant_core import AntGameFacade

def main() -> NoReturn:
    game = AntGameFacade()
    while True:
        game.update()

if __name__ == "__main__":
    main()
