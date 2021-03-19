#!/bin/bash
set -e
docker build --build-arg DOCKER_BASE=ubuntu:20.04 . -t gfootball_docker_test
docker run --gpus all -v /tmp/.X11-unix:/tmp/.X11-unix:rw --entrypoint bash -it gfootball_docker_test -c 'set -e; for x in `find gfootball/env -name *_test.py`; do UNITTEST_IN_DOCKER=1 PYTHONPATH=/ python3 $x; done'

docker build --build-arg DOCKER_BASE=ubuntu:18.04 . -t gfootball_docker_test -f Dockerfile_examples
docker run -v /tmp/.X11-unix:/tmp/.X11-unix:rw --gpus all --entrypoint python3 -it gfootball_docker_test gfootball/examples/run_ppo2.py --level=academy_empty_goal_close --num_timesteps=10000
echo "Test successful!!!"
