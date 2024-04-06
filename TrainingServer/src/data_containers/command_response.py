import dataclasses
from dataclasses import dataclass
import json


@dataclass
class AIClientResponse:
    event: str
    id: str
    isSuccess: bool
    step: int
    command: int

    def to_json(self, indent: int = 2) -> str:
        return json.dumps(dataclasses.asdict(self), indent=indent)
