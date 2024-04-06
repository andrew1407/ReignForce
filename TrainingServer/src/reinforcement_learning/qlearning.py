import tensorflow as tf
import keras
from keras import layers

import numpy as np
import math
import random

from collections import namedtuple, deque


Experience = namedtuple(typename='Experience', field_names=('state', 'action', 'next_state', 'reward', 'done'))


class ReplayMemory:
    def __init__(self, capacity: int):
        self.memory = deque([], maxlen=capacity)

    def push(self, experience: Experience):
        self.memory.append(experience)

    def sample(self, batch_size: int) -> list[Experience]:
        return random.sample(self.memory, batch_size)
    
    def can_provide_sample(self, batch_size: int) -> bool:
        return len(self) >= batch_size

    def __len__(self) -> int:
        return len(self.memory)


class EpsilonGreedyStrategy:
    def __init__(self, start: float, end: float, decay: float):
        self.start = start
        self.end = end
        self.decay = decay

    def get_exploration_rate(self, current_step: int) -> float:
        return EpsilonGreedyStrategy.exploration_rate(self.start, self.end, self.decay, current_step)
    
    def __call__(self, current_step: int) -> float:
        return self.get_exploration_rate(current_step)

    @staticmethod
    def exploration_rate(start: float, end: float, decay: float, current_step: int):
        return end + (start - end) * math.exp(-1 * current_step * decay)


def create_q_dnn_model(params: tuple[int, int]) -> keras.Sequential:
    inputs, putputs = params
    model = keras.Sequential((
        layers.InputLayer(input_shape=(inputs,)),
        layers.Dense(units=256, activation='relu'),
        layers.Dense(units=128, activation='relu'),
        layers.Dense(units=64, activation='relu'),
        layers.Dense(units=32, activation='relu'),
        layers.Dense(units=putputs),
    ))
    return model


def extract_tensors(experiences: list[Experience]) -> tuple[tf.Tensor, tf.Tensor, tf.Tensor, tf.Tensor, tf.Tensor]:
    batch = Experience(*zip(*experiences))
    t1 = tf.convert_to_tensor(batch.state)
    t2 = tf.convert_to_tensor(batch.action)
    t3 = tf.convert_to_tensor(batch.reward, dtype=tf.float32)
    t4 = tf.convert_to_tensor(batch.next_state)
    t5 = tf.convert_to_tensor(batch.done)
    return t1, t2, t3, t4, t5


def check_validity(done: tf.Tensor, rewards: tf.Tensor, reward_threshold: tf.Tensor) -> np.ndarray:
    reward_fits = tf.math.greater_equal(rewards, reward_threshold)
    not_finished = tf.math.logical_not(done)
    valid = tf.math.logical_and(reward_fits, not_finished)
    return valid


def get_next_q_max(target_net: keras.Model, next_states: np.ndarray, validity_mask: np.ndarray) -> tf.Tensor:
    non_final_states = tf.boolean_mask(next_states, validity_mask)
    batch_size = tf.shape(next_states)[0]
    values = tf.zeros(batch_size, dtype=tf.float32)

    if tf.reduce_any(validity_mask):
        max_q_values = tf.reduce_max(target_net(non_final_states), axis=1)
        invalid_indices = tf.where(validity_mask)  # Get the indices where valid is False
        invalid_indices_expanded = tf.squeeze(invalid_indices, axis=1)  # Squeeze the indices to shape (num_false_positions,)
        values = tf.tensor_scatter_nd_update(values, invalid_indices_expanded[:, tf.newaxis], max_q_values)

    return values
