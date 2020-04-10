import sys
sys.path.insert(0, '/app/football/')
import gfootball.env as football_env
print (football_env.__file__)

rewards = 'scoring,targShot'

env = football_env.create_environment(env_name="11_vs_11_easy_stochastic",
                            stacked=False, logdir='/tmp/football',
                            write_goal_dumps=False,
                            write_full_episode_dumps=False,
                            render=False,
                            rewards = rewards)

env.reset()
steps = 0
while True:
  obs, rew, done, info = env.step(env.action_space.sample())
  steps += 1
  if steps % 100 == 0:
    print("Step %d Reward: %f" % (steps, rew))
  if done:
    env.reset()
    print ("Done")
    exit()
print("Steps: %d Reward: %.2f" % (steps, rew))
