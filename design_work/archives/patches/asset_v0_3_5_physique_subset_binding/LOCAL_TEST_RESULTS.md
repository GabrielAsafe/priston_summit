# Local test results — V0.3.5

- `python -m compileall`: PASS.
- `smb_physique_closure_signature`:
  - dois SMBs byte-diferentes com o mesmo Root→Head retornaram a mesma assinatura;
  - skeleton sem o índice Physique solicitado falhou explicitamente.
- Registry/cache fixture `PHYSIQUE_SUBSET_EQUIVALENT`:
  - `binding_mode=ROOT_SKELETON_BOUND`;
  - `binding_skeleton_smb` preenchido;
  - `rigid_binding_ready=true`;
  - `animation_binding_ready=false`;
  - `visual_mode=RIGID`.
- Structural fixture usando somente `binding_skeleton_smb`: PASS.
- `asset motions` em binding Physique sem contexto de motion: bloqueado explicitamente.
- Helper Blender `extract_physique_usage.py`: compile-only neste ambiente; execução real será no Blender 5.1.1 do usuário.
- Nenhum arquivo do cliente é modificado.
