# PT Design Work Cleanup V1.1

Non-destructive root cleanup.

- dry-run performs no filesystem writes;
- apply preflights all paths before moving;
- no deletion;
- collisions receive timestamped destination names;
- completed package folders/ZIPs go to `archives/cleanup_<timestamp>/packages/`;
- old Python generations go to `history_py/`;
- misnamed legacy workspace is archived;
- audit text goes to `logs/audits/`;
- caches are archived;
- current production and next G3 tools remain in the root;
- apply writes `logs/cleanup_<timestamp>.tsv`.
