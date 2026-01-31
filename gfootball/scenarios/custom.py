from . import *

def build_scenario(builder):
  builder.config().game_duration = 3000
  builder.config().right_team_difficulty = 0.05
  builder.config().left_team_difficulty = 0.05
  builder.config().deterministic = False

  config = builder.Context()

  # Set up left team
  builder.SetTeam(Team.e_Left)
  if 'left_team_players' in config:
      for p in config['left_team_players']:
          builder.AddPlayer(p['x'], p['y'], p['role'], lazy=p.get('lazy', False), controllable=p.get('controllable', True), kit_no=p.get('kit_no', 0), player_stats=p.get('player_stats'))
  else:
      builder.AddPlayer(-1.000000, 0.000000, e_PlayerRole_GK)

  # Set up right team
  builder.SetTeam(Team.e_Right)
  if 'right_team_players' in config:
      for p in config['right_team_players']:
          builder.AddPlayer(p['x'], p['y'], p['role'], lazy=p.get('lazy', False), controllable=p.get('controllable', True), kit_no=p.get('kit_no', 0), player_stats=p.get('player_stats'))
  else:
      builder.AddPlayer(-1.000000, 0.000000, e_PlayerRole_GK)
