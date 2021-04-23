from gfootball.env.football_env import FootballEnvBase
from gfootball.env.football_env_core import FootballEnvCore
from party_env_core import PartyEnvCore


class PartyEnv(FootballEnvBase):

  def __init__(self, config):
    super().__init__(config)
    self._env = PartyEnv._get_environment_core(
            self._config['environment_core'])(self._config)

  _environment_cores = {
      'gfootball': FootballEnvCore,
      'party': PartyEnvCore,
  }

  def _get_environment_core(core_type):
    if core_type not in PartyEnv._environment_cores:
      raise Exception(f'Invalid environment_core: {core_type} . ' +
          'Expected one of: [{}]'.format(
              ', '.join(PartyEnv._environment_cores.keys())))

    return PartyEnv._environment_cores[core_type]
