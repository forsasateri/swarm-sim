# TODO

## Focus:
- Improve pathfinding for better performance
- Trim checks and logic for perfromance
    - Only update neighbours when entering new block
- Fix freese from pathfinding when target is blocked - currently just instant recalculation over entire search space
    - Maybe even add some huristic for giving up?
- Split last updated and last observed time for world model - improve staleness
- Max staleness time to prevent infinite belief

## Other
- Dynamic walls being added / removed
- Fix warnings
- Switch walkers to more direct state machine with clearer logic
- Separate pathfinding tools in own file


## Performance
- Handle run to far to reach target
    - If movement this tick > distance to target, just move to target and stop
- Fixed tick death spiral?
- Only observe on block change
- Route blockage scan still walks remaining route at each waypoint in randomWalker.cc:66.
- Local + global time meassuremnts to visualize