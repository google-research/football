import yaml

from gfootball.env import config


_PARTY_CONFIG_DEFAULTS_PATH = '../configs/defaults.yaml'

with open(_PARTY_CONFIG_DEFAULTS_PATH, 'r') as stream:
  _party_config_defaults = yaml.safe_load(stream)


class PartyConfig(config.Config):

  def __init__(self, values=None):
    # Will probably need to do some updating here eventually
    super().__init__(values)

  def _set_defaults(config, defaults):
    for key in defaults:
      if key not in config:
        config[key] = defaults[key]
      elif isinstance(config[key], dict):
        _set_defaults(config[key], defaults[key])

  def _convert_players(players):
    pass

  def _convert_yaml_dict(config):
    return {
        'action_set': config['game']['actionSet'],
        'custom_display_stats': config['display']['gameStats'],
        'dump_full_episodes': config['dump']['fullEpisodes'],
        'dump_scores': config['dump']['scores'],
        'players': _convert_players(config['players']),
        'level': config['game']['level'],
        'physics_steps_per_frame': config['game']['physicsStepsPerFrame'],
        'render_resolution_x': config['display']['renderResolution'],
        'real_time': config['game']['realTime'],
        'tracesdir': config['tracesdir'],
        'video_format': config['video']['format'],
        'video_quality_level': config['video']['quality'],
        'write_video': config['video']['write'],
    }

  def from_yaml(file_path):
    with open(file_path, 'r') as stream:
      yaml_data = yaml.safe_load(stream)

    _set_defaults(yaml_data, _party_config_defaults)

    config = _convert_yaml_dict(yaml_data)

    return PartyConfig(config)

