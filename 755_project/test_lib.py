from baselines.ppo2 import ppo2

import sys
sys.path.insert(0, '/app/football/')
import gfootball.env as football_env

print ("Test to make sure I am working with the correct lib")
print (sys.path)
print (football_env.__file__)

assert (football_env.__file__ == '/app/football/gfootball/env/__init__.py')

print ("Test passed")
