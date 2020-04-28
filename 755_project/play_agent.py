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
        pass

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


players = ["ppo2_cnn:left_players=1,policy=gfootball_impala_cnn,checkpoint={0}".format('ckpts/11_vs_11_easy_stochastic_v2')]
extra_vals = config.Config({'game_engine_random_seed' : 12})
cfg = config.Config({
  'action_set':'default',
  'dump_full_episodes': False,
  'real_time':False,
  'players' : players,
  'level':'11_vs_11_easy_deterministic',
  'game_engine_random_seed' : 12
})

env = football_env.FootballEnv(cfg)
env.reset()

obsDebugger = ObservationDebugger()

my_score = 0
opp_score = 0
step = 0

while True:
    obs, rew, done, info = env.step([])
    obsDebugger.add_observation(obs, env._get_actions()[0])
    #print (obs['game_mode'], obs['left_agent_controlled_player'], obs['ball_owned_team'])
    if (obs['game_mode'] == 3) and (obs['ball_owned_team'] == 0):
        exit()
    if rew == 1:
        my_score += 1

        obsDebugger.process_observation(step)
        exit()
    if rew == -1:
        opp_score += 1
        # obsDebugger.process_observation(step)
    if done:
        print (my_score, opp_score)
        my_score = 0
        opp_score = 0
        env.reset()

    step += 1
