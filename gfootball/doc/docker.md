# Running in Docker #
In order to build Docker image you have to checkout GFootball git repository first:

```
git clone https://github.com/google-research/football.git
cd football
```

## CPU version
1. Build with `docker build --build-arg DOCKER_BASE=ubuntu:18.04 --build-arg DEVICE=cpu . -t gfootball`
1. Enter the image with `docker run -e DISPLAY=$DISPLAY -it -v /tmp/.X11-unix:/tmp/.X11-unix:rw gfootball bash`

## GPU version
1. Build with `docker build --build-arg DOCKER_BASE=tensorflow/tensorflow:1.14.0-gpu-py3 --build-arg DEVICE=gpu . -t gfootball`
1. Enter the image with `nvidia-docker run -e DISPLAY=$DISPLAY -it -v /tmp/.X11-unix:/tmp/.X11-unix:rw gfootball bash` or `docker run --gpus all -e DISPLAY=$DISPLAY -it -v /tmp/.X11-unix:/tmp/.X11-unix:rw gfootball bash` for docker 19.03 or later.

Inside the Docker image you can interact with the environment the same way as in case of local installation.
For example, to play the game yourself you can run:

```
python3 -m gfootball.play_game --action_set=full
```

Or you can run a sample PPO2 training with:

```
python3 -m gfootball.examples.run_ppo2
```

## Building a docker image under MacOS

You may need to increase memory for building. Go to the docker menu, then
Preferences, then Advanced/Memory and set memory to the 4GB.
