import yaml

from gfootball.env import config


class PartyConfig(config.Config):

  def __init__(self, values=None):
    # Will probably need to do some updating here eventually
    super().__init__(values)

  def _convert_players(players):
    return ['{kind}:{side}_players={count}'.format(**player)
        for player in players]

  def _convert_yaml_dict(config):
    return {
        'action_set': config['game']['actionSet'],
        'custom_display_stats': config['display']['gameStats'],
        'dump_full_episodes': config['dump']['fullEpisodes'],
        'dump_scores': config['dump']['scores'],
        'environment_core': config['env']['core'],
        'level': config['game']['level'],
        'physics_steps_per_frame': config['game']['physicsStepsPerFrame'],
        'players': PartyConfig._convert_players(config['players']),
        'render_resolution_x': config['display']['renderResolution'],
        'real_time': config['game']['realTime'],
        'tracesdir': config['tracesDir'],
        'video_format': config['video']['format'],
        'video_quality_level': config['video']['quality'],
        'write_video': config['video']['write'],
    }

  def from_yaml(file_path):
    with open(file_path, 'r') as stream:
      yaml_data = yaml.safe_load(stream)

    config = PartyConfig._convert_yaml_dict(yaml_data)

    return PartyConfig(config)

