# TODO

## Focus:
- Improve pathfinding for better performance
- Trim checks and logic for perfromance
    - Only update neighbours when entering new block
- Fix freese from pathfinding when target is blocked - currently just instant recalculation over entire search space
    - Maybe even add some huristic for giving up?
- Split last updated and last observed time for world model - improve staleness

## Other
- Dynamic walls being added / removed
- Fix warnings
- Switch walkers to more direct state machine with clearer logic
- Separate pathfinding tools in own file