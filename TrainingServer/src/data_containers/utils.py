from typing import Union
import json

from .command_request import CommandRequest, ClientInput, ShooterMetrics
from .command_response import AIClientResponse
from .command_request import CommandRequest
from .shooter_command import ShooterAICommand
from .training_progress import TrainingProgress


def shooter_data_to_metrics(shooter_metrics: ShooterMetrics) -> list[float]:
    return list(map(float, (
        shooter_metrics.shooterState,
        shooter_metrics.isMoving,
        shooter_metrics.healthPercent,
        shooter_metrics.isFirearmsActive,
        shooter_metrics.activeWeapon,
    )))


def client_input_to_metrics(client_input: ClientInput) -> list[float]:
    core_input = list(map(float, (
        client_input.isTargetVisible,
        client_input.command,
        client_input.distanceState,
    )))
    return core_input + shooter_data_to_metrics(client_input.selfMetrics) + shooter_data_to_metrics(client_input.targetMetrics)


def create_default_respoonse(request_body: CommandRequest) -> AIClientResponse:
    return AIClientResponse(
        event=request_body.event,
        id=request_body.id,
        isSuccess=True,
        step=request_body.step,
        command=ShooterAICommand.NoneCommand.value,
    )


def create_invlid_respoonse(request_body: CommandRequest) -> AIClientResponse:
    respoonse = create_default_respoonse(request_body)
    respoonse.isSuccess = False
    return respoonse


def snap_state(progress: TrainingProgress, request_body: CommandRequest) -> tuple[dict[str, Union[bool, int, float, str]], str]:
        state = {
            'step (total)': progress.step,
            'episode': progress.episode,
            'step (per episode):': progress.step_per_episode,
            'reward (per episode)': progress.reward,
            'reward (per step)': request_body.clientInput.reward,
            'reward / spe': progress.reward_spe_relation(),
            'command': request_body.clientInput.command,
            'done': request_body.clientInput.isDone,
        }
        return state, json.dumps(state, indent=2)
