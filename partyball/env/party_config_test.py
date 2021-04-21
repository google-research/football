import party_config


def test_default_constructor():
  config = party_config.PartyConfig()
  assert config['action_set'] == 'default'


def test_yaml_constructor():
  config = party_config.PartyConfig.from_yaml('../configs/default.yaml')
  assert config['action_set'] == 'default'
  assert config['players'] == ['agent:left_players=1',
      'keyboard:right_players=1']
