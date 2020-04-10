import sys
sys.path.insert(0, '/app/football/')
from gfootball.env import football_env
from gfootball.env import config

print (football_env.__file__)

players = ["ppo2_cnn:left_players=1,policy=gfootball_impala_cnn,checkpoint={0}".format('ckpts/11_vs_11_easy_stochastic_v2')]

cfg = config.Config({
  'action_set':'default',
  'dump_full_episodes': False,
  'real_time':False,
  'players' : players,
  'level':'11_vs_11_easy_stochastic'
})

env = football_env.FootballEnv(cfg)
env.reset()

my_score = 0
opp_score = 0

while True:
    _, rew, done, info = env.step([])
    if rew == 1:
        my_score += 1
    if rew == -1:
        opp_score += 1
    if done:
        print (my_score, opp_score)
        my_score = 0
        opp_score = 0
        env.reset()
        exit()
