#!/bin/bash

python3  run_ppo2.py \
  --level 'academy_counterattack_easy' \
  --reward_experiment 'scoring,passing' \
  --policy impala_cnn \
  --cliprange 0.115 \
  --gamma 0.997 \
  --ent_coef 0.00155 \
  --num_timesteps 50000000 \
  --max_grad_norm 0.76 \
  --lr 0.00011879 \
  --num_envs 2 \
  --noptepochs 2 \
  --nminibatches 4 \
  --nsteps 512 \
  "$@"

# Needed to add: max_grad_norm

# Good but unsettable defaults:
# Optimizer: adam
# Value-function coefficient is 0.5
# GAE (lam): 0.95
