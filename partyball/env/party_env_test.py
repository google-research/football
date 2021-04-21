from gfootball.env.football_env_core import FootballEnvCore

from party_env import PartyEnv
from party_env_core import PartyEnvCore
from party_config import PartyConfig


def test_party_env():
    config = PartyConfig.from_yaml('../configs/default.yaml')
    env = PartyEnv(config)
    assert isinstance(env._env, PartyEnvCore)


def test_default_env():
    config = PartyConfig.from_yaml('../configs/default.yaml')
    config['environment_core'] = 'gfootball'
    env = PartyEnv(config)
    assert isinstance(env._env, FootballEnvCore)
