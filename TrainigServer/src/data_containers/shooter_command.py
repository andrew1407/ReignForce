from enum import Enum


class ShooterAICommand(Enum):
    NoneCommand = 0
    Idle = 1
    IdleAndAttack = 2
    GetClose = 3
    GetCloseAndAttack = 4
    StrafeAttack = 5
    StrafeAttackAndAttack = 6
    Retreat = 7

    @classmethod
    def from_value(cls, value: int):
        for el in cls:
            if el.value == value: return el
        return cls.NoneCommand
