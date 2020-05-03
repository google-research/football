import sys
sys.path.insert(0, '/app/football/')
from gfootball.env import football_env
from gfootball.env import config
import matplotlib.pyplot as plt
import numpy as np

print (football_env.__file__)

#O -> my team (left)
#1 -> opposing team (right)
class TeamPlotter:
    def __init__(self, which_team):
        self.which_team = which_team
        self.xys = {0: [], 1: [], 2: [], 3: [],
                    4: [], 5: [], 6: [], 7: [],
                    8: [], 9: [], 10: []}
        self.team_key = '{}_team'.format(which_team)

    def record_positions(self, obs):
        team_pos = obs[self.team_key]
        for idx, pos in enumerate(team_pos):
            self.xys[idx].append(pos)


players = ["ppo2_cnn:left_players=1,policy=impala_cnn,checkpoint={0}".format('checkpoints/01200')]
extra_vals = config.Config({'game_engine_random_seed' : 12})
cfg = config.Config({
  'action_set':'default',
  'dump_full_episodes': False,
  'real_time':False,
  'players' : players,
  'level':'11_vs_11_easy_stochastic',
  'game_engine_random_seed' : 12
})

env = football_env.FootballEnv(cfg)
env.reset()

l_team_plotter = TeamPlotter(which_team = 'left')
r_team_plotter = TeamPlotter(which_team = 'right')

step =0
zs = []

while True:
    obs, rew, done, info = env.step([])
    left_goalie = obs['left_team'][0]
    right_goalie = obs['right_team'][0]
    #ball_pos = obs['ball']
    #z = ball_pos[2]
    #zs.append(z)
    print (left_goalie[0])
    #l_team_plotter.record_positions(obs)
    #r_team_plotter.record_positions(obs)
    step +=1
    if done:
        env.reset()
        exit()
    

# heights = np.asarray(zs)
# np.save('heights', heights)

# heights = np.load('heights.npy')
# print (heights)
# print (np.max(heights))
#
# _ = plt.hist(heights, bins='auto')
# plt.show()
# plt.savefig('heights_hist')
# exit()

left_goalie_movements = np.asarray(l_team_plotter.xys[0])
right_goalie_movements = np.asarray(r_team_plotter.xys[0])

np.save('left_goalie_movements', left_goalie_movements)
np.save('right_goalie_movements', right_goalie_movements)


left_goalie_movements = np.load('left_goalie_movements.npy')
left_max_x = np.max(left_goalie_movements[:,0])
left_min_x = np.min(left_goalie_movements[:,0])
left_max_y = np.max(left_goalie_movements[:,1])
left_min_y = np.min(left_goalie_movements[:,1])
print (left_max_x, left_min_x, left_max_y, left_min_y)
print ((left_max_y + left_min_y) / 2)

print ("h")
