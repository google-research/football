# Google Research Football

This repository contains an RL environment based on open-source game Gameplay
Football. It was created by the Google Brain team for research purposes.

This is not an official Google product.

We'd like to thank Bastiaan Konings Schuiling, who authored and open-sourced the original version of this game.

For more information, please look at our [paper (github)](https://github.com/google-research/football/blob/master/paper.pdf).
Mailing list: https://groups.google.com/forum/#!forum/google-research-football

## Installation
Currently we're supporting only Linux and Python3.
You can either install the code from github (newest version) or from pypi (stable version).

  1. Install required apt packages with
  `sudo apt-get install git cmake build-essential libgl1-mesa-dev libsdl2-dev
  libsdl2-image-dev libsdl2-ttf-dev libsdl2-gfx-dev libboost-all-dev
  libdirectfb-dev libst-dev mesa-utils xvfb x11vnc libsqlite3-dev
  glee-dev libsdl-sge-dev python3-pip`

  1. Install gfootball python package from pypi:

    - Use `pip3 install gfootball[tf_cpu] --process-dependency-links` if you want to use CPU version of TensorFlow.
    - Use `pip3 install gfootball[tf_gpu] --process-dependency-links` if you want to use GPU version of TensorFlow.
    - This command can run for couple of minutes, as it compiles the C++ environment in the background.

  OR install gfootball python package (run the commands from the main project directory):

    - `git clone https://github.com/google-research/football.git`
    - `cd football`
    - Use `pip3 install .[tf_cpu] --process-dependency-links` if you want to use the CPU version of TensorFlow.
    - Use `pip3 install .[tf_gpu] --process-dependency-links` if you want to use GPU version of TensorFlow.
    - This command can run for a couple of minutes, as it compiles the C++ environment in the background.

## Running experiments
- To run example PPO experiment on `academy_empty_goal` scenario, run
`python3 -m gfootball.examples.run_ppo2 --level=academy_empty_goal_close`
- To run on `academy_pass_and_shoot_with_keeper` scenario, run
`python3 -m gfootball.examples.run_ppo2 --level=academy_pass_and_shoot_with_keeper`

In order to train with nice replays being saved, run
`python3 -m gfootball.examples.run_ppo2 --dump_full_episodes=True --render=True`

## Playing game yourself
Run `python3 -m gfootball.play_game`. By default, it starts the
base scenario and the home player is controlled by the keyboard. Different types
of players are supported (gamepad, external bots, agents...). For possible
options run `python3 -m gfootball.play_game -helpfull`.

Please note that playing
the game is implemented through an environment, so human-controlled players use
the same interface as the agents. One important fact is that there is a single
action per 100 ms reported to the environment, which might cause a lag effect
when playing.

## Multiagent support
Using play_game script (see 'Playing game yourself' section for details)
it is possible to set up a game played between multiple agents.
`home_players` and `away_players` command line parameters are the comma-separated list of players on the home and away teams, respectively.
For example, to play yourself using a gamepad with two lazy bots on your team
against three bots you can run
`python3 -m gfootball.play_game --home_players=gamepad,lazy,lazy --away_players=bot,bot,bot`.

You can implement your own player by adding its implementation
to the env/players directory (no other changes are needed).
Have a look at existing players code for an example implementation.

### Keyboard mapping
The game defines following keyboard mapping:

- `ARROW UP` - run to the top.
- `ARROW DOWN` - run to the bottom.
- `ARROW LEFT` - run to the left.
- `ARROW RIGHT` - run to the right.
- `S` - short pass in attack mode, pressure in the defense mode.
- `A` - high pass in attack mode, sliding in the defense mode.
- `D` - shot in the attack mode, team pressure in the defense mode.
- `W` - long pass in the attack mode, goalkeeper pressure in the defense mode.
- `Q` - switch the active player on defense mode.
- `C` - dribble in the attack mode.
- `E` - sprint.

## Running in Docker
### CPU version
1. Build with `docker build --build-arg DOCKER_BASE=ubuntu:18.04 --build-arg DEVICE=cpu . -t gfootball`
1. Enter the image with `docker run -it gfootball bash`

### GPU version
1. Build with `docker build --build-arg DOCKER_BASE=tensorflow/tensorflow:1.12.0-gpu-py3 --build-arg DEVICE=gpu . -t gfootball`
1. Enter the image with `nvidia-docker run -it gfootball bash`

After entering the image, you can run sample training with `python3 -m gfootball.examples.run_ppo2`.
Unfortunately, rendering is not supported inside the docker.

## Observations
Environment exposes following observations:

- Ball information:
    - `ball` - [x, y, z] position of the ball.
    - `ball_direction` - [x, y, z] ball movement vector.
    - `ball_rotation` - [x, y, z] rotation angles in radians.
    - `ball_owned_team` - {-1, 0, 1}, -1 = ball not owned, 0 = home team, 1 = away team.
    - `ball_owned_player` - {0..N-1} integer denoting index of the player owning the ball.
- Home team:
    - `home_team` - N-elements vector with [x, y] positions of players.
    - `home_team_direction` - N-elements vector with [x, y] movement vectors of players.
    - `home_team_tired_factor` - N-elements vector of floats in the range {0..1}. 0 means player is not tired at all.
    - `home_team_yellow_card` - N-elements vector of integers denoting number of yellow cards a given player has (0 or 1).
    - `home_team_active` - N-elements vector of Bools denoting whether a given player is playing the game (False means player got a red card).
    - `home_team_roles` - N-elements vector denoting roles of players. The meaning is:
        - `0` = e_PlayerRole_GK - goalkeeper,
        - `1` = e_PlayerRole_CB - centre back,
        - `2` = e_PlayerRole_LB - left back,
        - `3` = e_PlayerRole_RB - right back,
        - `4` = e_PlayerRole_DM - defence midfield,
        - `5` = e_PlayerRole_CM - central midfield,
        - `6` = e_PlayerRole_LM - left midfield,
        - `7` = e_PlayerRole_RM - right midfield,
        - `8` = e_PlayerRole_AM - attack midfield,
        - `9` = e_PlayerRole_CF - central front,
- Away team:
    - `away_team` - same as for home team.
    - `away_team_direction` - same as for home team.
    - `away_team_tired_factor` - same as for home team.
    - `away_team_yellow_card` - same as for home team.
    - `away_team_active` - same as for home team.
    - `away_team_roles` - same as for home team.
- Controlled player information:
    - `active` - {0..N-1} integer denoting index of the controlled player.
    - `sticky_actions` - 11-elements vector of 0s or 1s denoting whether corresponding actions are active:
        - `game_left`
        - `game_top_left`
        - `game_top`
        - `game_top_right`
        - `game_right`
        - `game_bottom_right`
        - `game_bottom`
        - `game_bottom_left`
        - `game_sprint`
        - `game_keeper_rush`
        - `game_dribble`
- Match state:
    - `score` - pair of integers denoting number of goals for home and away teams, respectively.
    - `steps_left` - how many steps are left till the end of the match.
    - `game_mode` - current game mode, one of:
        - `0` = `e_GameMode_Normal`
        - `1` = `e_GameMode_KickOff`
        - `2` = `e_GameMode_GoalKick`
        - `3` = `e_GameMode_FreeKick`
        - `4` = `e_GameMode_Corner`
        - `5` = `e_GameMode_ThrowIn`
        - `6` = `e_GameMode_Penalty`
- Screen:
    - `frame` - three 1280x720 vectors of RGB pixels representing rendered
    screen. It is only exposed when rendering is enabled (`render` flag).

Where `N` is the number of players on the team.
X coordinates are in the range `[-1, 1]`.
Y coordinates are in the range `[-0.42, 0.42]`.
Speed vectors represent a change in the position of the object within a single
step.

## Scenarios
We provide two sets of scenarios/levels:

- Football Benchmarks
   - 11_vs_11_stochastic - full 90 minutes football game (medium difficulty)
   - 11_vs_11_easy_stochastic - full 90 minutes football game (easy difficulty)
   - 11_vs_11_hard_stochastic - full 90 minutes football game (hard difficulty)

- Football Academy - with a total of 11 scenarios
   - academy_empty_goal_close - Our player starts inside the box with the ball,
     and needs to score against an empty goal.
   - academy_empty_goal - Our player starts in the middle of the field with the
     ball, and needs to score against an empty goal.
   - academy_run_to_score - Our player starts in the middle of the field with
     the ball, and needs to score against an empty goal. Five opponent players
     chase ours from behind.
   - academy_run_to_score_with_keeper - Our player starts in the middle of the
     field with the ball, and needs to score against a keeper. Five opponent
     players chase ours from behind.
   - academy_pass_and_shoot_with_keeper - Two of our players try to score from
     the edge of the box, one is on the side with the ball, and next to a
     defender. The other is at the center, unmarked, and facing the opponent
     keeper.
   - academy_run_pass_and_shoot_with_keeper -  Two of our players try to score
     from the edge of the box, one is on the side with the ball, and unmarked.
     The other is at the center, next to a defender, and facing the opponent
     keeper.
   - academy_3_vs_1_with_keeper - Three of our players try to score from the
     edge of the box, one on each side, and the other at the center. Initially,
     the player at the center has the ball and is facing the defender.
     There is an opponent keeper.
   - academy_corner - Standard corner-kick situation, except that the corner
     taker can run with the ball from the corner.
   - academy_counterattack_easy - 4 versus 1 counter-attack with keeper; all the
     remaining players of both teams run back towards the ball.
   - academy_counterattack_hard - 4 versus 2 counter-attack with keeper; all the
     remaining players of both teams run back towards the ball.
   - academy_single_goal_versus_lazy - Full 11 versus 11 games, where the
     opponents cannot move but they can only intercept the ball if it is close
     enough to them. Our center back defender has the ball at first.

You can add your own scenarios by adding a new file to the `gfootball/scenarios/`
directory. Have a look at existing scenarios for example.

## Trace dumps
GFootball environment supports the recording of scenarios for later watching or
analysis. Each trace dump consists of a picked episode trace (observations,
reward, additional debug info) and optionally an AVI file with the rendered episode.
Picked episode trace can be played back later on using `replay.py` script.
By default trace, dumps are disabled to not occupy disk space. They
are controlled by the following set of flags:

-  `dump_full_episodes` - should trace for each entire episode be recorded.
-  `dump_scores` - should sample traces for scores be recorded.
-  `tracesdir` - directory in which trace dumps are saved.
-  `write_video` - should a video be recorded together with the trace.
    If rendering is disabled (`render` config flag), the video contains a simple
    episode animation.

## Frequent Problems & Solutions

### Rendering not working / "OpenGL version not equal to or higher than 3.2"

Solution: set environment variables for MESA driver, like this:

`MESA_GL_VERSION_OVERRIDE=3.2 MESA_GLSL_VERSION_OVERRIDE=150 python3 -m gfootball.play_game`

### process-dependency-links flag not found

See: https://github.com/google-research/football/issues/1

### Building a docker image under MacOS

You may need to increase memory for building. Go to the docker menu, then
Preferences, then Advanced/Memory and set memory to the 4GB.
