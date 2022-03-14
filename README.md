# Google Research Football

This repository contains an RL environment based on open-source game Gameplay
Football. <br> It was created by the Google Brain team for research purposes.

Useful links:

* [Run in Colab](https://colab.research.google.com/github/google-research/football/blob/master/gfootball/colabs/gfootball_example_from_prebuild.ipynb) - start training in less that 2 minutes.
* [Google Research Football Paper](https://arxiv.org/abs/1907.11180)
* [GoogleAI blog post](https://ai.googleblog.com/2019/06/introducing-google-research-football.html)
* [Google Research Football on Cloud](https://towardsdatascience.com/reproducing-google-research-football-rl-results-ac75cf17190e)
* [GRF Kaggle competition](https://www.kaggle.com/c/google-football) - take part in the competition playing games against others, win prizes and become the GRF Champion!


We'd like to thank Bastiaan Konings Schuiling, who authored and open-sourced the original version of this game.


## Quick Start

### In colab

Open our example [Colab](https://colab.research.google.com/github/google-research/football/blob/master/gfootball/colabs/gfootball_example_from_prebuild.ipynb), that will allow you to start training your model in less than 2 minutes.

This method doesn't support game rendering on screen - if you want to see the game running, please use the method below.

### Using Docker

This is the recommended way for Linux-based systems to avoid incompatible package versions.
Instructions are available [here](gfootball/doc/docker.md).

### On your computer

#### 1. Install required packages
#### Linux
```shell
sudo apt-get install git cmake build-essential libgl1-mesa-dev libsdl2-dev \
libsdl2-image-dev libsdl2-ttf-dev libsdl2-gfx-dev libboost-all-dev \
libdirectfb-dev libst-dev mesa-utils xvfb x11vnc python3-pip

python3 -m pip install --upgrade pip setuptools psutil wheel
```

#### macOS
First install [brew](https://brew.sh/). It should automatically install Command Line Tools.
Next install required packages:

```shell
brew install git python3 cmake sdl2 sdl2_image sdl2_ttf sdl2_gfx boost boost-python3

python3 -m pip install --upgrade pip setuptools psutil wheel
```


#### Windows
Install [Git](https://git-scm.com/download/win) and [Python 3](https://www.python.org/downloads/).
Update pip in the Command Line (here and for the **next steps** type `python` instead of `python3`)
```commandline
python -m pip install --upgrade pip setuptools psutil wheel
```


#### 2. Install GFootball
#### Option a. From PyPi package (recommended)
```shell
python3 -m pip install gfootball
```

#### Option b. Installing from sources using GitHub repository 
(On Windows you have to install additional tools and set an environment variable, see 
[Compiling Engine](gfootball/doc/compile_engine.md#windows) for detailed instructions.)

```shell
git clone https://github.com/google-research/football.git
cd football
```

Optionally you can use [virtual environment](https://docs.python.org/3/tutorial/venv.html):

```shell
python3 -m venv football-env
source football-env/bin/activate
```

Next, build the game engine and install dependencies:

```shell
python3 -m pip install .
```
This command can run for a couple of minutes, as it compiles the C++ environment in the background.
If you face any problems, first check [Compiling Engine](gfootball/doc/compile_engine.md) documentation and search GitHub issues.


#### 3. Time to play!
```shell
python3 -m gfootball.play_game --action_set=full
```
Make sure to check out the [keyboard mappings](#keyboard-mappings).
To quit the game press Ctrl+C in the terminal.

# Contents #

* [Running training](#training-agents-to-play-GRF)
* [Playing the game](#playing-the-game)
    * [Keyboard mappings](#keyboard-mappings)
    * [Play vs built-in AI](#play-vs-built-in-AI)
    * [Play vs pre-trained agent](#play-vs-pre-trained-agent)
    * [Trained checkpoints](#trained-checkpoints)
* [Environment API](gfootball/doc/api.md)
* [Observations & Actions](gfootball/doc/observation.md)
* [Scenarios](gfootball/doc/scenarios.md)
* [Multi-agent support](gfootball/doc/multi_agent.md)
* [Running in docker](gfootball/doc/docker.md)
* [Saving replays, logs, traces](gfootball/doc/saving_replays.md)
* [Imitation Learning](gfootball/doc/imitation.md)

## Training agents to play GRF

### Run training
In order to run TF training, you need to install additional dependencies

- Update PIP, so that tensorflow 1.15 is available: `python3 -m pip install --upgrade pip setuptools wheel`
- TensorFlow: `python3 -m pip install tensorflow==1.15.*` or
  `python3 -m pip install tensorflow-gpu==1.15.*`, depending on whether you want CPU or
  GPU version;
- Sonnet and psutil: `python3 -m pip install dm-sonnet==1.* psutil`;
- OpenAI Baselines:
  `python3 -m pip install git+https://github.com/openai/baselines.git@master`.

Then:

- To run example PPO experiment on `academy_empty_goal` scenario, run
  `python3 -m gfootball.examples.run_ppo2 --level=academy_empty_goal_close`
- To run on `academy_pass_and_shoot_with_keeper` scenario, run
  `python3 -m gfootball.examples.run_ppo2 --level=academy_pass_and_shoot_with_keeper`

In order to train with nice replays being saved, run
`python3 -m gfootball.examples.run_ppo2 --dump_full_episodes=True --render=True`

In order to reproduce PPO results from the paper, please refer to:

- gfootball/examples/repro_checkpoint_easy.sh
- gfootball/examples/repro_scoring_easy.sh

## Playing the game

Please note that playing the game is implemented through an environment, so human-controlled players use the same interface as the agents.
One important implication is that there is a single action per 100 ms reported to the environment, which might cause a lag effect when playing.


### Keyboard mappings
The game defines following keyboard mapping (for the `keyboard` player type):

* `ARROW UP` - run to the top.
* `ARROW DOWN` - run to the bottom.
* `ARROW LEFT` - run to the left.
* `ARROW RIGHT` - run to the right.
* `S` - short pass in the attack mode, pressure in the defense mode.
* `A` - high pass in the attack mode, sliding in the defense mode.
* `D` - shot in the attack mode, team pressure in the defense mode.
* `W` - long pass in the attack mode, goalkeeper pressure in the defense mode.
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

  - [11_vs_11_easy_stochastic](https://storage.googleapis.com/gfootball/11_vs_11_easy_stochastic_v2),
  - [academy_run_to_score_with_keeper](https://storage.googleapis.com/gfootball/academy_run_to_score_with_keeper_v2).

In order to see the checkpoints playing, run
`python3 -m gfootball.play_game --players "ppo2_cnn:left_players=1,policy=gfootball_impala_cnn,checkpoint=$CHECKPOINT" --level=$LEVEL`,
where `$CHECKPOINT` is the path to downloaded checkpoint. Please note that the checkpoints were trained with Tensorflow 1.15 version. Using 
different Tensorflow version may result in errors. The easiest way to run these checkpoints is through provided `Dockerfile_examples` image.
See [running in docker](gfootball/doc/docker.md) for details (just override the default Docker definition with `-f Dockerfile_examples` parameter).

In order to train against a checkpoint, you can pass 'extra_players' argument to create_environment function.
For example extra_players='ppo2_cnn:right_players=1,policy=gfootball_impala_cnn,checkpoint=$CHECKPOINT'.
