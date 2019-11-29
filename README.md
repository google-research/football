# Google Research Football

This repository contains an RL environment based on open-source game Gameplay
Football. <br> It was created by the Google Brain team for research purposes.

Useful links:
* __(NEW!)__ [GRF Game Server](https://research-football.dev/) - challenge other researchers!
* [Google Research Football Paper](https://arxiv.org/abs/1907.11180)
* [GoogleAI blog post](https://ai.googleblog.com/2019/06/introducing-google-research-football.html)
* [Google Research Football on Cloud](https://towardsdatascience.com/reproducing-google-research-football-rl-results-ac75cf17190e)
* [Mailing List](https://groups.google.com/forum/#!forum/google-research-football) - please use it for communication with us (comments / suggestions / feature ideas)


For non-public matters that you'd like to discuss directly with the GRF team,
please use google-research-football@google.com.

We'd like to thank Bastiaan Konings Schuiling, who authored and open-sourced the original version of this game.


## Quick Start
Install required apt packages with:

```
sudo apt-get install git cmake build-essential libgl1-mesa-dev libsdl2-dev \
libsdl2-image-dev libsdl2-ttf-dev libsdl2-gfx-dev libboost-all-dev \
libdirectfb-dev libst-dev mesa-utils xvfb x11vnc libsdl-sge-dev python3-pip
```

Then install the game from GitHub master:
```
git clone https://github.com/google-research/football.git
cd football
pip3 install .
```
This command can run for a couple of minutes, as it compiles the C++ environment in the background. Now, it's time to play!

```
python3 -m gfootball.play_game --action_set=full
```

# Contents #

* [Running experiments](#training-agents-to-play-GRF)
* [Playing the game](#playing-the-game)
    * [Keyboard mappings](#keyboard-mappings)
    * [Play vs built-in AI](#play-vs-built-in-AI)
    * [Play vs pre-trained agent](#play-vs-pre-trained-agent)
    * [Trained checkpoints](#trained-checkpoints)
* [Environment API](gfootball/doc/api.md)
* [Observations](gfootball/doc/observation.md)
* [Scenarios](gfootball/doc/scenarios.md)
* [Multi-agent support](gfootball/doc/multi_agent.md)
* [Running in docker](gfootball/doc/docker.md)
* [Saving replays, logs, traces](gfootball/doc/saving_replays.md)

## Training agents to play GRF

### Run training
In order to run TF training, install additional dependencies:

- TensorFlow: `pip3 install "tensorflow<2.0"` or
  `pip3 install "tensorflow-gpu<2.0"`, depending on whether you want CPU or
  GPU version;
- Sonnet: `pip3 install dm-sonnet`;
- OpenAI Baselines:
  `pip3 install git+https://github.com/openai/baselines.git@master`.

Then:

- To run example PPO experiment on `academy_empty_goal` scenario, run
  `python3 -m gfootball.examples.run_ppo2 --level=academy_empty_goal_close`
- To run on `academy_pass_and_shoot_with_keeper` scenario, run
  `python3 -m gfootball.examples.run_ppo2 --level=academy_pass_and_shoot_with_keeper`

In order to train with nice replays being saved, run
`python3 -m gfootball.examples.run_ppo2 --dump_full_episodes=True --render=True`


## Playing the game

Please note that playing the game is implemented through an environment, so human-controlled players use the same interface as the agents. One important implication is that there is a single action per 100 ms reported to the environment, which might cause a lag effect when playing.


### Keyboard mappings
The game defines following keyboard mapping (for the `keyboard` player type):

* `ARROW UP` - run to the top.
* `ARROW DOWN` - run to the bottom.
* `ARROW LEFT` - run to the left.
* `ARROW RIGHT` - run to the right.
* `S` - short pass in the attack mode, pressure in the defense mode.
* `A` - high pass in the attack mode, sliding in the defense mode.
* `D` - shot in the the attack mode, team pressure in the defense mode.
* `W` - long pass in the the attack mode, goalkeeper pressure in the defense mode.
* `Q` - switch the active player in the defense mode.
* `C` - dribble in the attack mode.
* `E` - sprint.

### Play vs built-in AI
Run `python3 -m gfootball.play_game --action_set=full`. By default, it starts
the base scenario and the left player is controlled by the keyboard. Different
types of players are supported (gamepad, external bots, agents...). For possible
options run `python3 -m gfootball.play_game -helpfull`.

### Play vs pre-trained agent

In particular, one can play against agent trained with `run_ppo2` script with
the following command (notice no action_set flag, as PPO agent uses default
action set):
`python3 -m gfootball.play_game --players "keyboard:left_players=1;ppo2_cnn:right_players=1,checkpoint=$YOUR_PATH"`

### Trained checkpoints
We provide trained PPO checkpoints for the following scenarios:

  - [11_vs_11_easy_stochastic](https://storage.googleapis.com/gfootball-public-bucket/trained_model_11_vs_11_easy_stochastic),
  - [academy_run_to_score_with_keeper](https://storage.googleapis.com/gfootball-public-bucket/trained_model_academy_run_to_score_with_keeper).

In order to see the checkpoints playing, run
`python3 -m gfootball.play_game --players "ppo2_cnn:left_players=1,policy=gfootball_impala_cnn,checkpoint=$CHECKPOINT" --level=$LEVEL`,
where `$CHECKPOINT` is the path to downloaded checkpoint.

In order to train against a checkpoint, you can pass 'extra_players' argument to create_environment function.
For example extra_players='ppo2_cnn:right_players=1,policy=gfootball_impala_cnn,checkpoint=$CHECKPOINT'.

## Frequent Problems & Solutions

### Rendering not working / "OpenGL version not equal to or higher than 3.2"

Solution: set environment variables for MESA driver, like this:

`MESA_GL_VERSION_OVERRIDE=3.2 MESA_GLSL_VERSION_OVERRIDE=150 python3 -m gfootball.play_game`
