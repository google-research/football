# Multiagent support #

Using play_game script (see 'Playing game yourself' section for details)
it is possible to set up a game played between multiple agents.
`players` command line parameter is a comma-separated lists of players for both teams.
For example, to play yourself using a gamepad with two lazy bots on your team
against three bots you can run
`python3 -m gfootball.play_game --players=gamepad:left_players=1;lazy:left_players=2;bot:right_players=1;bot:right_players=1;bot:right_players=1`.

Notice the use of `left_players=2` for the lazy player, bot player does not support it.

You can implement your own player controlling multiple players by adding its implementation
to the env/players directory (no other changes are needed).
Have a look at existing players code for an example implementation.

To train a policy controlling multiple players, one has to do the following:
- pass `number_of_players_agent_controls` to the 'create_environment' defining how many players you want to control
- instead of calling '.step' function with a single action, call it with an array of actions, one action per player

It is up to the caller to unpack/postprocess it in a desirable way.
A simple example of training multi-agent can be found in examples/run_multiagent_rllib.py

