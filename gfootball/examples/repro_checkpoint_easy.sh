#!/bin/bash

python3 -u -m gfootball.examples.run_ppo2 \
  --level 11_vs_11_easy_stochastic \
  --reward_experiment scoring,checkpoints \
  --policy impala_cnn \
  --cliprange 0.08 \
  --gamma 0.993 \
  --ent_coef 0.003 \
  --num_timesteps 50000000 \
  --max_grad_norm 0.64 \
  --lr 0.000343 \
  --num_envs 16 \
  --noptepochs 2 \
  --nminibatches 8 \
  --nsteps 512 \
  "$@"
