import gfootball.env as football_env
# env = football_env.create_environment(env_name="tests.11_vs_11_hard_deterministic", stacked=False, logdir='/tmp/football', write_goal_dumps=True, write_full_episode_dumps=True, render=False)
env = football_env.create_environment(
  env_name='tests.multiagent_wrapper',
  rewards='checkpoints,scoring',
  representation='simple115v2',
  number_of_left_players_agent_controls=11,
  number_of_right_players_agent_controls=11,
  logdir='/tmp/football',
  write_goal_dumps=True,
  write_full_episode_dumps=True,
  render=False
)
env.reset()
steps = 0
while True:
  obs, rew, done, info = env.step(env.action_space.sample())
  if done:
    break

# python3 ./gfootball/dump_to_txt.py --trace_file=/tmp/football/episode_done_20210816-171444709588.dump --output=/tmp/dump.txt
# python3 ./gfootball/dump_to_video.py --trace_file=/tmp/football/episode_done_20210816-171444709588.dump