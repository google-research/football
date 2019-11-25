# Running in Docker #
## CPU version
1. Build with `docker build --build-arg DOCKER_BASE=ubuntu:18.04 --build-arg DEVICE=cpu . -t gfootball`
1. Enter the image with `docker run -it gfootball bash`

## GPU version
1. Build with `docker build --build-arg DOCKER_BASE=tensorflow/tensorflow:1.12.0-gpu-py3 --build-arg DEVICE=gpu . -t gfootball`
1. Enter the image with `nvidia-docker run -it gfootball bash` or `docker run --gpus all -it gfootball bash` for docker 19.03 or later.

After entering the image, you can run sample training with `python3 -m gfootball.examples.run_ppo2`.
Unfortunately, rendering is not supported inside the docker.

Note that tensorflow/tensorflow:1.12.0-gpu-py3 is based on Ubuntu 16.04 with python 3.5. If you want python 3.6, you should instead use tensorflow/tensorflow:1.14.0-gpu-py3.

## Building a docker image under MacOS

You may need to increase memory for building. Go to the docker menu, then
Preferences, then Advanced/Memory and set memory to the 4GB.
