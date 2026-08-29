# G6 Manual All-LOD Gate V1.2

Fixes the V1.1 wrapper regression:

    NameError: name 'DEFAULT_ARTIST' is not defined

The parser already supported `--artist`, but the module-level default retained
the old variable name `ARTIST`. V1.2 defines `DEFAULT_ARTIST` consistently.

The package also runs the CLI with `--help` as a smoke test before delivery,
so unresolved module-level parser names are caught.

All strict G6 gates remain unchanged:
- pristine client;
- all three targets manually changed;
- material 0 only;
- SMD_UV;
- 11 target vertex groups;
- six C01 non-targets unchanged;
- no force-test;
- multi-object writer;
- six SMD non-target blocks byte-identical;
- independent C002 High/Default/Low reimports;
- material audit;
- no automatic deployment.

`--artist` may point directly to a preserved `.blend1`.
