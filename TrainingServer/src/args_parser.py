import argparse


class ArgsParser:
    def __init__(self, default_host: str = '', default_port: int = 0, default_reward_threshold: float = 1):
        parser = argparse.ArgumentParser(description='Command line arguments parser')
    
        parser.add_argument('-a', '--addr', type=str, default=default_host, help='Host address')
        parser.add_argument('-p', '--port', type=int, default=default_port, help='Port number')

        parser.add_argument('-hp', '--hide_plot', action="store_true", help='Hides plot window')
        parser.add_argument('-cl', '--clear_log', action="store_true", help='Clears output log on update')

        parser.add_argument('-ml', '--model_label', type=str, default='', help='Adds label to models name')
        parser.add_argument('-rt', '--reward_threshold', type=float, default=default_reward_threshold, help='Validation threshold to filter input rewards')

        parser.add_argument('-tn', '--target_net', type=str, default='', help='tarrget_net model path to laod')
        parser.add_argument('-pn', '--policy_net', type=str, default='', help='policy_net model path to laod')

        self.__args: argparse.Namespace = parser.parse_args()
    

    def get_args(self) -> argparse.Namespace:
        return self.__args
    
    
    def get_connection_params(self) -> tuple[str, int]:
        return self.__args.addr, self.__args.port
    

    def get_plot_display_mode(self) -> bool:
        return not self.__args.hide_plot
    

    def get_name_label(self) -> str:
        return self.__args.model_label
    

    def get_reward_threshold(self) -> float:
        return self.__args.reward_threshold


    def get_models_to_load(self) -> tuple[str, str]:
        return self.__args.target_net, self.__args.policy_net


    def get_clear_log(self) -> bool:
        return self.__args.clear_log
