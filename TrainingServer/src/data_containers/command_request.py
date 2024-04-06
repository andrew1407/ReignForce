from dataclasses import dataclass
import json


@dataclass
class ShooterMetrics:
    shooterState: int
    isMoving: bool
    health: float
    healthPercent: float
    isFirearmsActive: bool
    activeWeapon: int

    @classmethod
    def from_json(cls, data: dict):
        return cls(**data)


@dataclass
class ClientInput:
    reward: int
    isDone: bool
    isTargetVisible: bool
    command: int
    targetDistance: float
    distanceState: int
    selfMetrics: ShooterMetrics
    targetMetrics: ShooterMetrics

    @classmethod
    def from_json(cls, data: dict):
        self_metrics_data = data.pop('selfMetrics', {})
        target_metrics_data = data.pop('targetMetrics', {})
        return cls(
            selfMetrics=ShooterMetrics.from_json(self_metrics_data),
            targetMetrics=ShooterMetrics.from_json(target_metrics_data),
            **data,
        )


@dataclass
class CommandRequest:
    event: str
    id: str
    isInitial: bool
    step: int
    clientInput: ClientInput
    previousClientInput: ClientInput

    @classmethod
    def from_json(cls, data: str):
        body = cls(**json.loads(data))
        body.clientInput = ClientInput.from_json(body.clientInput)
        body.previousClientInput = ClientInput.from_json(body.previousClientInput)
        return body
