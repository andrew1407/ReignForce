from typing import Callable
import websockets as ws

from data_containers.command_request import CommandRequest
from data_containers.command_response import AIClientResponse
from data_containers.shooter_command import ShooterAICommand
from data_containers.training_progress import TrainingProgress
import data_containers.utils as dcutils

from reinforcement_learning.action_trainer import ActionTrainer
from reinforcement_learning.plot_generator import PlotGenerator
from logger.qlearning_logger import QLearningLogger


INPUT_DATA_COMMAND = 'inputs-data'

MessageHandler = Callable[[CommandRequest], AIClientResponse]

class ClientEventsHandler:
    def __init__(self, action_trainer: ActionTrainer, plot_generator: PlotGenerator, logger: QLearningLogger, weights_update_rate: int, plot_update_rate: int = 1):
        self.__action_trainer = action_trainer
        self.__plot_generator = plot_generator
        self.__logger = logger
       
        self.__weights_update_rate = weights_update_rate
        self.__plot_update_rate = plot_update_rate

        self.__training_progress = TrainingProgress(step=0, reward=0, episode=-1, step_per_episode=0)
        self.__message_handlers: dict[str, MessageHandler] = {
            INPUT_DATA_COMMAND: lambda rqst: (self.__handle_initial_inputs_data if rqst.isInitial else self.__handle_inputs_data)(rqst)
        }
    

    def get_proress(self) -> tuple[int, int]:
        return max(self.__training_progress.episode, 0), max(self.__training_progress.step, 0)
    

    async def on_connect(self, websocket: ws.WebSocketServerProtocol, path: str):
        print(f"Client connected: {websocket.remote_address}")


    async def on_disconnect(self, websocket: ws.WebSocketServerProtocol, path: str):
        print(f"Client disconnected: {websocket.remote_address}")
        self.__training_progress.drop_episode_step()
        self.__training_progress.drop_reward()


    async def on_message(self, message: str, websocket: ws.WebSocketServerProtocol, path: str):
        request_body = CommandRequest.from_json(message)
        try:
            handler = self.__message_handlers.get(request_body.event)
            if not handler:
                raise Exception(f'caannot handle "{request_body.event}" event: no handler found')
            response_body = handler(request_body)
        except Exception as e:
            self.__logger.show_error(e)
            response_body = dcutils.create_invlid_respoonse(request_body)
        finally:
            await websocket.send(response_body.to_json())
    

    def __should_update_weights(self) -> bool:
        return self.__training_progress.episode % self.__weights_update_rate == 0
    

    def __should_update_plot(self) -> bool:
        return self.__training_progress.episode % self.__plot_update_rate == 0
    

    def __handle_initial_inputs_data(self, request_body: CommandRequest) -> AIClientResponse:
        response_body = dcutils.create_default_respoonse(request_body)
        input_metrics = dcutils.client_input_to_metrics(request_body.clientInput)

        predicted = self.__action_trainer.get_action(current_step=self.__training_progress.step, state=input_metrics)
        response_body.command = ShooterAICommand.from_value(predicted).value
        response_body.isSuccess = True

        self.__training_progress.acc_episode()
        self.__logger.show_training_step_result(self.__training_progress, request_body)
        self.__training_progress.acc_step()
        self.__training_progress.acc_reward(request_body.clientInput.reward)

        return response_body


    def __handle_inputs_data(self, request_body: CommandRequest) -> AIClientResponse:
        response_body = dcutils.create_default_respoonse(request_body)
        input_metrics = dcutils.client_input_to_metrics(request_body.clientInput)
        prev_input_metrics = dcutils.client_input_to_metrics(request_body.previousClientInput)
        
        self.__action_trainer.add_sample(
            state=prev_input_metrics,
            action=request_body.clientInput.command,
            next_state=input_metrics,
            reward=request_body.clientInput.reward,
            done=request_body.clientInput.isDone,
        )
        self.__action_trainer.train_by_sampple()

        self.__training_progress.acc_reward(request_body.clientInput.reward)
        self.__logger.show_training_step_result(self.__training_progress, request_body)

        if request_body.clientInput.isDone:
            self.__training_progress.drop_episode_step()

            if self.__should_update_weights():
                self.__action_trainer.update_model_parameters()

            if self.__should_update_plot():
                self.__plot_generator.update_history(to_add=self.__training_progress.reward_spe_relation())
                self.__plot_generator.update_plot(self.__training_progress.episode, self.__training_progress.step)

        predicted = self.__action_trainer.get_action(current_step=self.__training_progress.step, state=input_metrics)
        response_body.command = ShooterAICommand.from_value(predicted).value
        response_body.isSuccess = True

        self.__training_progress.acc_step()
    
        return response_body
