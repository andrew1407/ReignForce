from dataclasses import dataclass


@dataclass
class TrainingProgress:
    step: int = 0
    reward: float = 0
    episode: int = 0
    step_per_episode: int = 0

    def acc_step(self):
        self.step += 1
        self.step_per_episode += 1

    def acc_episode(self):
        self.episode += 1

    def acc_reward(self, value: float):
        self.reward += value

    def drop_episode_step(self):
        self.step_per_episode = 0

    def drop_reward(self):
        self.reward = 0
    
    def spe_relation_safe(self) -> int:
        return max(self.step_per_episode, 1)
    
    def reward_spe_relation(self) -> float:
        return self.reward / self.spe_relation_safe()
