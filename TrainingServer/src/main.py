import os
from dataclasses import dataclass
import asyncio

from reinforcement_learning.action_trainer import ActionTrainer
from reinforcement_learning.plot_generator import PlotGenerator

from ws_server.training_server import TrainingServer
from ws_server.client_events_handler import ClientEventsHandler
from args_parser import ArgsParser
from logger.qlearning_logger import QLearningLogger


PORT = 8080
HOST = '127.0.0.1'

SHOOTER_METRICS_COUNT = 5
SELF_METRICS_COUNT = SHOOTER_METRICS_COUNT
TARGET_METRICS = SHOOTER_METRICS_COUNT
GENERAL_INPUT_PARAMS_COUNT = 3

MODEL_INPUTS_COUNT = SELF_METRICS_COUNT + TARGET_METRICS + GENERAL_INPUT_PARAMS_COUNT
MODEL_OUTPUTS_COUNT = 8

BATCH_SIZE = 256
GAMMA = .999
EPS_START = 1
EPS_END = .01
EPS_DECAY = .001

TARGET_MODEL_UPDATE = 10
MEMORY_SIZE = 500
LEARNIING_RATE = .001
# DISPLAY_PLOTS = True

REWARD_VALIDATION_THRESHOLD = 1

OUT_DIR = 'out/'
POLICY_MODEL_PATH = OUT_DIR + 'policy_net'
TARGET_MODEL_PATH = OUT_DIR +'target_net'
PLOT_PATH = OUT_DIR +'progress-plot'


@dataclass
class VariblesContext:
    args_parser: ArgsParser
    action_trainer: ActionTrainer
    plot_generator: PlotGenerator
    client_events_handler: ClientEventsHandler
    server: TrainingServer
    logger: QLearningLogger


def create_variables_context() -> VariblesContext:
    args_parser = ArgsParser(default_host=HOST, default_port=PORT, default_reward_threshold=REWARD_VALIDATION_THRESHOLD)
    logger = QLearningLogger(clear_on_update=args_parser.get_clear_log())
    action_trainer = ActionTrainer(replay_memory_size=MEMORY_SIZE, batch_size=BATCH_SIZE, learning_rate=LEARNIING_RATE, gamma=GAMMA,
                                   reward_threshold=args_parser.get_reward_threshold(), eps=(EPS_START, EPS_END, EPS_DECAY),
                                   model_params=(MODEL_INPUTS_COUNT, MODEL_OUTPUTS_COUNT), model_paths=args_parser.get_models_to_load())
    plot_generator = PlotGenerator(title='Training', display=args_parser.get_plot_display_mode())
    events_handler = ClientEventsHandler(action_trainer=action_trainer, plot_generator=plot_generator, logger=logger,
                                         weights_update_rate=TARGET_MODEL_UPDATE, plot_update_rate=1)
    server = TrainingServer(events_handler=events_handler)
    return VariblesContext(
        args_parser=args_parser,
        action_trainer=action_trainer,
        plot_generator=plot_generator,
        client_events_handler=events_handler,
        server=server,
        logger=logger,
    )


def on_close(variables_context: VariblesContext):
    episodes, steps = variables_context.client_events_handler.get_proress()
    if not os.path.exists(OUT_DIR): os.mkdir(OUT_DIR)

    appendix = '_%s_%d_%d' % (variables_context.args_parser.get_name_label(), episodes, steps)
    policy_path = POLICY_MODEL_PATH + appendix
    target_path = TARGET_MODEL_PATH + appendix
    plot_path = PLOT_PATH + appendix

    variables_context.action_trainer.save_models(policy=policy_path, target=target_path)
    variables_context.plot_generator.save(filename=plot_path)


async def main(variables_context: VariblesContext):
    host, port = variables_context.args_parser.get_connection_params()
    await variables_context.server.run(host=host, port=port)


if __name__ == '__main__':
    variables_context = create_variables_context()
    try:
        asyncio.run(main(variables_context))
    except (KeyboardInterrupt, EOFError):
        print('Closing...')
        on_close(variables_context)
