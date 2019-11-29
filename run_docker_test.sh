#!/bin/bash
set -e
docker build --build-arg DOCKER_BASE=ubuntu:18.04 --build-arg DEVICE=cpu . -t gfootball_docker_test
docker run --entrypoint bash -it gfootball_docker_test -c 'set -e; for x in `find gfootball -name *_test.py`; do UNITTEST_IN_DOCKER=1 PYTHONPATH=/ python3 $x; done'
docker run --entrypoint python3 -it gfootball_docker_test gfootball/examples/run_ppo2.py --level=academy_empty_goal_close --num_timesteps=10000
echo "Test successful!!!"
