import os

from data_containers.command_request import CommandRequest
from data_containers.training_progress import TrainingProgress
import data_containers.utils as dcutils


class QLearningLogger:
    def __init__(self, clear_on_update: bool = False):
        self.clear_on_update = clear_on_update


    def show_training_step_result(self, progress: TrainingProgress, request_body: CommandRequest):
        _, to_log = dcutils.snap_state(progress, request_body)

        if self.clear_on_update:
            clear_cmd = 'cls' if os.name == 'nt' else 'clear'
            os.system(clear_cmd)

        print(to_log, '\n')
        if request_body.clientInput.isDone and not self.clear_on_update:
            print('======================================================', '\n')
    

    def show_error(self, exception: Exception):
        red_color_code = '\033[91m'
        reset_color_code = '\033[0m'
        red_wrap = f'{red_color_code}%s{reset_color_code}'

        msg = red_wrap % f'Error: {exception}'
        print(msg)
