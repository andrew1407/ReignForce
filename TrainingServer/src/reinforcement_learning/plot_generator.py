import numpy as np
import matplotlib
import matplotlib.pyplot as plt


class PlotGenerator:
    def __init__(self, title: str, display: bool = True):
        self.__history: list[float] = list()
        self.set_display_mode(display)
        self.__title = title

        plt.ion()
        self.__fig, self.__ax = plt.subplots()
        self.__line, = self.__ax.plot([], [])
        self.__ax.set_xlabel('Episodes')
        self.__ax.set_ylabel('Reward / Steps per episode')
        self.update_plot()


    def set_display_mode(self, display: bool):
        matplotlib.use('TkAgg' if display else 'Agg')


    def update_plot(self, episode: int = 0, step: int = 0, step_per_episode: int = 0):
        title = '%s (episode: %d; step (per episode): %d; step (total): %d)' % (self.__title, episode, step_per_episode, step)
        self.__ax.set_title(title)

        if self.__history:
            y = self.__history
            x = np.arange(len(y))
            self.__line.set_data(x, y)
        else:
            self.__line.set_data([], [])

        self.__ax.relim()
        self.__ax.autoscale_view()
        plt.pause(0.01)

    
    def update_history(self, to_add: float):
        self.__history.append(to_add)

    
    def save(self, filename: str):
        fmt = 'png'
        save_to = filename if filename.endswith(f'.{fmt}') else f'{filename}.{fmt}'
        self.__fig.savefig(save_to, format=fmt)
