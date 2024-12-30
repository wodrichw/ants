from typing import NoReturn
import sys
from ant_core import AntGameFacade

def main() -> NoReturn:
    game = AntGameFacade()
    while True:
        game.update()

if __name__ == "__main__":
    main()
