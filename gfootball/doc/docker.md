# Running in Docker #
In order to build Docker image you have to checkout GFootball git repository first:

```
git clone https://github.com/google-research/football.git
cd football
```

For **rendering** the game on macOS and Windows, we recommend installing the game according to
the instructions for your platform in [README](https://github.com/google-research/football#on-your-computer).

## Configure Docker for Rendering (Linux only)
In order to see rendered game you need to allow Docker containers access X server:

```
xhost +"local:docker@"
```

This command has to be executed after each reboot. Alternatively you can add this
command to `/etc/profile` to not worry about it in the future.

## Build Docker image

### Tensorflow without GPU-training support version

```
docker build --build-arg DOCKER_BASE=ubuntu:20.04 . -t gfootball
```

### Tensorflow with GPU-training support version

```
docker build --build-arg DOCKER_BASE=tensorflow/tensorflow:1.15.2-gpu-py3 . -t gfootball
```

## Start the Docker image

```
docker run --gpus all -e DISPLAY=$DISPLAY -it -v /tmp/.X11-unix:/tmp/.X11-unix:rw gfootball bash
```

If you get errors related to `--gpus all` flag, you can replace it with `--device /dev/dri/[X]`
adding this flag for every file in the `/dev/dri/` directory. It makes sure that GPU is
visible inside the Docker image. You can also drop it altogether (environment
will try to perform software rendering).

## Run environment

Inside the Docker image you can interact with the environment the same way as in case of local installation.
For example, to play the game yourself you can run:

```
python3 -m gfootball.play_game --action_set=full
```

To run example PPO2 training you need to install OpenAI Baselines in addition.

## Building a docker image under MacOS

You may need to increase memory for building. Go to the docker menu, then
Preferences, then Advanced/Memory and set memory to the 4GB.
