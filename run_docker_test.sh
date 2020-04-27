#!/bin/bash
set -e
#docker build --build-arg DEVICE=gpu . -t gfootball_docker_test
echo "Test successful!!!"


docker run -it --runtime=nvidia --ipc=host --net=host \
       --privileged \
       --env display \
       --volume /playpen/john/docker_scripts/.vimrc:/root/.vimrc \
       --volume /tmp/.x11-unix:/tmp/.x11-unix \
       --volume /var/run/docker.sock:/var/run/docker.sock \
       --env xauthority=/root/.xauthority \
       -v /playpen/john/ML/755_proj/:/app \
       -v /net/vision29/data/c/:/data \
       --env nvidia_driver_capabilities=compute,utility \
       gfootball_docker_test:latest bash

