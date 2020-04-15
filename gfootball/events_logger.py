from enum import Enum
import json
from typing import List


class EventCode(Enum):
    goal = "GOAL"
    corner = "CORNER"
    offside = "OFFSIDE"
    substitution = "SUBSTITUTION"
    yellow_card = "YELLOW_CARD"
    red_card = "RED_CARD"
    shot = "SHOT_ON_TARGET"
    kick_off = "KICK_OFF"
    end_half = "HALF_TIME_FINISHED"
    end_game = "FULL_TIME_FINISHED"


class Event(Enum):
    team_id: int
    time_in_sec: int
    message: str
    event_code: EventCode


class EventsLogger:
    _events: List[Event] = []

    def __init__(self, match_id: str) -> None:
        self._match_id = match_id

    def _save_events_to_file(self) -> None:
        events_to_save = []
        for event in self._events:
            events_to_save.append({
                "gameevent": {
                    "event": event.event_code.value, "info": [event.message], "match_id": self._match_id,
                    "sport": "football", "t": "gameevent", "team": event.team_id,
                    "timer": {"direction": "ASC", "state": "TICKING", "value": event.time_in_sec}},
                "ts": event.time_in_sec
            })
        with open(f"gameevents_{self._match_id}.json", "w") as f:
            json.dump(events_to_save, f)

    def log_event(self, event: Event) -> None:
        time = f"{event.time_in_sec // 60:02d}:{event.time_in_sec % 60:02d}"
        print(f"{time} {event.message}")
        self._events.append(event)
        if event.event_code == EventCode.end_game:
            self._save_events_to_file()
