PT_PIKEMAN_RESET_PRISTINE_FULL_ASSET_ONE_COMMAND_V1_1

Fix:
V1 created the temporary Blender Python helper under host /tmp.
The Flatpak Blender process has a different /tmp namespace and could not open it.

V1.1 creates the helper under:
  ~/Área de Trabalho/summit/design_work/sh/.pt_pikeman_auto_uv_g7_XXXXXX.py

That location is visible to Blender because the command uses --filesystem=home.

All strict reset/build/deploy behavior from V1 is unchanged.
It is safe to rerun the full script; existing intermediate active files are
archived before the new pristine run.
