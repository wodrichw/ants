from typing import List


class ReplayError:
    message: str
    expected_version: int
    actual_version: int
    frame_index: int
    event_index: int
    event_kind: str


class AntGameFacade:
    def __init__(self) -> None:
        pass

    def update(self) -> bool:
        pass

    def engine_update(self) -> None:
        pass

    def start_replay_recording(self, path: str) -> bool:
        pass

    def start_replay_playback(self, path: str) -> bool:
        pass

    def stop_replay_recording(self) -> None:
        pass

    def replay_has_error(self) -> bool:
        pass

    def replay_last_error(self) -> ReplayError:
        pass

    def replay_done(self) -> bool:
        pass

    def action_move_player(self, dx: int, dy: int) -> None:
        pass

    def action_dig(self, dx: int, dy: int) -> None:
        pass

    def action_create_ant(self) -> None:
        pass

    def action_add_program_lines(self, lines: List[str]) -> None:
        pass

    def action_assign_program_to_ant(self, ant_idx: int) -> None:
        pass

    def action_go_up(self) -> None:
        pass

    def action_go_down(self) -> None:
        pass
