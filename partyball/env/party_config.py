import yaml

from gfootball.env import config


class PartyConfig(config.Config):

  def __init__(self, values=None):
    # Will probably need to do some updating here eventually
    super().__init__(values)

  def from_yaml(file_path):
    with open(file_path, 'r') as stream:
      yaml_data = yaml.safe_load(stream)

    party_config = PartyConfig()

    return PartyConfig()
