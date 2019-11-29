# Scenarios #
We provide two sets of scenarios/levels:

* Football Benchmarks
   * __11_vs_11_stochastic__ A full 90 minutes football game (medium difficulty)
   * __11_vs_11_easy_stochastic__ A full 90 minutes football game (easy difficulty)
   * __11_vs_11_hard_stochastic__ A full 90 minutes football game (hard difficulty)

* Football Academy - with a total of 11 scenarios
   * __academy_empty_goal_close__ - Our player starts inside the box with the ball,
     and needs to score against an empty goal.
   * __academy_empty_goal__ - Our player starts in the middle of the field with the
     ball, and needs to score against an empty goal.
   * __academy_run_to_score__ - Our player starts in the middle of the field with
     the ball, and needs to score against an empty goal. Five opponent players
     chase ours from behind.
   * __academy_run_to_score_with_keeper__ - Our player starts in the middle of the
     field with the ball, and needs to score against a keeper. Five opponent
     players chase ours from behind.
   * __academy_pass_and_shoot_with_keeper__ - Two of our players try to score from
     the edge of the box, one is on the side with the ball, and next to a
     defender. The other is at the center, unmarked, and facing the opponent
     keeper.
   * __academy_run_pass_and_shoot_with_keeper__ -  Two of our players try to score
     from the edge of the box, one is on the side with the ball, and unmarked.
     The other is at the center, next to a defender, and facing the opponent
     keeper.
   * __academy_3_vs_1_with_keeper__ - Three of our players try to score from the
     edge of the box, one on each side, and the other at the center. Initially,
     the player at the center has the ball and is facing the defender.
     There is an opponent keeper.
   * __academy_corner__ - Standard corner-kick situation, except that the corner
     taker can run with the ball from the corner.
   * __academy_counterattack_easy__ - 4 versus 1 counter-attack with keeper; all the
     remaining players of both teams run back towards the ball.
   * __academy_counterattack_hard__ - 4 versus 2 counter-attack with keeper; all the
     remaining players of both teams run back towards the ball.
   * __academy_single_goal_versus_lazy__ - Full 11 versus 11 games, where the
     opponents cannot move but they can only intercept the ball if it is close
     enough to them. Our center back defender has the ball at first.

You can add your own scenarios by adding a new file to the `gfootball/scenarios/`
directory. Have a look at existing scenarios for example.
