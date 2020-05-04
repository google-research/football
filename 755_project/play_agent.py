import sys
sys.path.insert(0, '/app/football/')
from gfootball.env import football_env
from gfootball.env import config

print (football_env.__file__)

#O -> my team (left)
#1 -> opposing team (right)
class ObservationDebugger:
    def __init__(self):

        #only conerned with left player
        self.observations = {} #key,value -> step, observation
        self.step_ct = 0


    def process_observation(self, step):

        this_obs, this_action = self.observations[step]
        this_ball_owned_team = this_obs['ball_owned_team']
        this_ball_owned_player = this_obs['left_agent_controlled_player']
        print (this_ball_owned_team, this_ball_owned_player, step, this_obs['score'],  this_action )
        for i in range(30):
            prev_obs, prev_action = self.observations[step-i-1]
            prev_ball_owned_team = prev_obs['ball_owned_team']
            prev_ball_owned_player = prev_obs['left_agent_controlled_player']
            print (prev_ball_owned_team, prev_ball_owned_player, step-i-1, prev_obs['score'], prev_action)
        exit()


    def add_observation(self, obs, action):
        self.observations[self.step_ct] = (obs, action)
        self.step_ct += 1

class Rectangle(object):
    def __init__(self, xrange, yrange, zrange):
        self.xrange = xrange  # (xmin, xmax)
        self.yrange = yrange
        self.zrange = zrange

    def contains_point(self, p):
        if not all(hasattr(p, loc) for loc in 'xyz'):
            raise TypeError("Can only check if 3D points are in the rect")
        return all([self.xrange[0] <= p.x <= self.xrange[1],
                    self.yrange[0] <= p.y <= self.yrange[1],
                    self.zrange[0] <= p.z <= self.zrange[1]])

class Point(object):
    def __init__(self, x, y ,z):
        self.x = x
        self.y = y
        self.z = z

    def __iter__(self):
        yield from (self.x, self.y, self.z)

    def __str__(self):
        return "str {} {} {}".format(self.x, self.y, self.z)

ckpt_path = 'corner_ckpt_all/00200'
players = ["ppo2_cnn:left_players=1,policy=impala_cnn,checkpoint={0}".format(ckpt_path)]
cfg = config.Config({
  'action_set':'default',
  'dump_full_episodes': False,
  'real_time':False,
  'players' : players,
  'level':'academy_pass_and_shoot_with_keeper'
})

env = football_env.FootballEnv(cfg)

env.reset()

obsDebugger = ObservationDebugger()

my_score = 0
opp_score = 0
step = 0
total_diff = 0.0
total_eps = 0
Opponent_GOAL = Rectangle(xrange = (.7, 1.1), yrange = (-.12,.12), zrange = (0, 2.5))

while True:
    obs, rew, done, info = env.step([])

    ball_pos = obs['ball']
    # ball_point = Point(ball_pos[0], ball_pos[1], ball_pos[2])
    # ball_on_targ = Opponent_GOAL.contains_point(ball_point)
    # if not rew == 0:
    #     print (rew)
    #     print (info)
    #     exit()
    if rew == 1.0:
        my_score += 1
    if rew == -1.0:
        opp_score += 1

    if done:
        diff = my_score - opp_score

        total_diff += diff
        my_score = 0
        opp_score = 0
        env.reset()

        total_eps += 1
        if total_eps == 100:

            break
print (total_diff)
print (total_diff/total_eps)
print (ckpt_path)
