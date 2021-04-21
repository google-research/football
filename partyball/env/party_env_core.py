from gfootball.env.football_env_core import FootballEnvCore


class PartyEnvCore(FootballEnvCore):

  def __init__(self, config):
    super().__init__(config)

  def compute_reward(self):
    score_diff = self._observation['score'][0] - self._observation['score'][1]
    return score_diff - self._state.previous_score_diff
