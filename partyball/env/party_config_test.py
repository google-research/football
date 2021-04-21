import party_config


def default_constructor():
  config = PartyConfig()
  assert config['action_set'] == 'default'


def yaml_constructor():
  config = PartyConfig.from_yaml('../configs/default.yaml')
  assert config['action_set'] == 'default'
  assert config['players'] == ['agent:left_players=1',
      'keyboard:right_players=1']
