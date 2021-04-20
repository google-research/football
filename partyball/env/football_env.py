from gfootball.env import football_env


class PartyEnv(football_env.FootballEnvBase):

  def __init__(self, config):
    super().__init__(config)
    self._env = _get_environment_core(
            self._config['environment_core'])(self._config)

  _environment_cores = {
      'base': football_env_core.FootballEnvCore,
      'party': football_env_core.PartyEnvCore,
  }

  def _get_environment_core(core_type):
    if core_type not in _environment_cores:
      raise Exception(f'Invalid environment_core: {core_type} . ' +
          'Expected one of: [{}]'.format(', '.join(_environment_cores.keys())))

    return _environment_cores[core_type]
