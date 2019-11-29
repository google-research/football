# Observation #

Environment exposes following `raw` observations:

- Ball information:
    - `ball` - [x, y, z] position of the ball.
    - `ball_direction` - [x, y, z] ball movement vector.
    - `ball_rotation` - [x, y, z] rotation angles in radians.
    - `ball_owned_team` - {-1, 0, 1}, -1 = ball not owned, 0 = left team, 1 = right team.
    - `ball_owned_player` - {0..N-1} integer denoting index of the player owning the ball.
- Left team:
    - `left_team` - N-elements vector with [x, y] positions of players.
    - `left_team_direction` - N-elements vector with [x, y] movement vectors of players.
    - `left_team_tired_factor` - N-elements vector of floats in the range {0..1}. 0 means player is not tired at all.
    - `left_team_yellow_card` - N-elements vector of integers denoting number of yellow cards a given player has (0 or 1).
    - `left_team_active` - N-elements vector of Bools denoting whether a given player is playing the game (False means player got a red card).
    - `left_team_roles` - N-elements vector denoting roles of players. The meaning is:
        - `0` = e_PlayerRole_GK - goalkeeper,
        - `1` = e_PlayerRole_CB - centre back,
        - `2` = e_PlayerRole_LB - left back,
        - `3` = e_PlayerRole_RB - right back,
        - `4` = e_PlayerRole_DM - defence midfield,
        - `5` = e_PlayerRole_CM - central midfield,
        - `6` = e_PlayerRole_LM - left midfield,
        - `7` = e_PlayerRole_RM - right midfield,
        - `8` = e_PlayerRole_AM - attack midfield,
        - `9` = e_PlayerRole_CF - central front,
- Right team:
    - `right_team` - same as for left team.
    - `right_team_direction` - same as for left team.
    - `right_team_tired_factor` - same as for left team.
    - `right_team_yellow_card` - same as for left team.
    - `right_team_active` - same as for left team.
    - `right_team_roles` - same as for left team.
- Controlled player information:
    - `active` - {0..N-1} integer denoting index of the controlled players.
    - `sticky_actions` - 10-elements vectors of 0s or 1s denoting whether corresponding action is active:
        - `0` - `game_left`
        - `1` - `game_top_left`
        - `2` - `game_top`
        - `3` - `game_top_right`
        - `4` - `game_right`
        - `5` - `game_bottom_right`
        - `6` - `game_bottom`
        - `7` - `game_bottom_left`
        - `8` - `game_sprint`
        - `9` - `game_dribble`
- Match state:
    - `score` - pair of integers denoting number of goals for left and right teams, respectively.
    - `steps_left` - how many steps are left till the end of the match.
    - `game_mode` - current game mode, one of:
        - `0` = `e_GameMode_Normal`
        - `1` = `e_GameMode_KickOff`
        - `2` = `e_GameMode_GoalKick`
        - `3` = `e_GameMode_FreeKick`
        - `4` = `e_GameMode_Corner`
        - `5` = `e_GameMode_ThrowIn`
        - `6` = `e_GameMode_Penalty`
- Screen:
    - `frame` - three 1280x720 vectors of RGB pixels representing rendered
    screen. It is only exposed when rendering is enabled (`render` flag).

Where `N` is the number of players on the team.
X coordinates are in the range `[-1, 1]`.
Y coordinates are in the range `[-0.42, 0.42]`.
Speed vectors represent a change in the position of the object within a single
step. In case of controlling `M` players (see multi-agent section), environment returns a list
of `M` observations, one per each controlled player. For convenience, even if you control players
on the right team, observations are mirrored (center of your goal is at `[-1, 0]`).

In addition, environment provides wrappers which convert `raw` observations to a different form:

- `simple115` (aka Simple115StateWrapper) - simplified representation of a game state encoded with 115 floats:
    - 44 = 11 (players) * 2 (x and y) * 2 (teams) - coordinates of players
    - 44 - player directions
    - 3 (x, y and z) - ball position
    - 3 - ball direction
    - 3 - one hot encoding of ball ownership (noone, left, right)
    - 11 - one hot encoding of which player is active
    - 7 - one hot encoding of `game_mode`
- `extracted` (aka SMMWrapper) - simplified spacial representation of a game state.
  It consists of several 72 * 96 planes ob bytes, filled in with 0s except for:
   - 1st plane: 255s represent positions of players on the left team
   - 2nd plane: 255s represent positions of players on the right team
   - 3rd plane: 255s represent positions of a ball
   - 4th plane: 255s represent positions of an active player
- `pixels`/`pixels_gray` (aka PixelsStateWrapper) - pixel representation, downscaled to 72 * 96, and converted to a single grayscale channel for `pixels_gray`.
  In order to use this representation you have to enable rendering in `create_environment` call (render=True parameter)
  or call `render` method on the environment object (in case of not using `create_environment`).

For trivial integration with single agent/player learning algorithms we provide a SingleAgentWrapper wrapper,
that strips the first dimension of the representation in the case of only one player being controlled.
