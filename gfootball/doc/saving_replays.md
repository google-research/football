# Saving replays, logs, traces #

GFootball environment supports recording of scenarios for later watching or
analysis. Each trace dump consists of a pickled episode trace (observations,
reward, additional debug info) and optionally an AVI file with the rendered episode.
Pickled episode trace can be played back later on using `replay.py` script.
By default trace dumps are disabled to not occupy disk space. They
are controlled by the following set of flags:

-  `dump_full_episodes` - should trace for each entire episode be recorded.
-  `dump_scores` - should sample traces for scores be recorded.
-  `tracesdir` - directory in which trace dumps are saved.
-  `write_video` - should a video be recorded together with the trace.
    If rendering is disabled (`render` config flag), the video contains a simple
    episode animation.

There are following scripts provided to operate on trace dumps:

-  `dump_to_txt.py` - converts trace dump to human-readable form.
-  `dump_to_video.py` - converts trace dump to a 2D representation video.
-  `replay.py` - replays a given trace dump using environment.

## Environment logs
Environment uses `absl.logging` module for logging.
You can change logging level by setting --verbosity flag to one of the following values:

-  `-1` - warning, only warnings and above are logged when problems are encountered,
-  `0` - info (the default), some per-episode statistics and similar are logged as well,
-  `1` - debug, additional debugging messages are included.

