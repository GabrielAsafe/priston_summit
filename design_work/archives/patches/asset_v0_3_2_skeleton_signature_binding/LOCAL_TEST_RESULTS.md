# Local test results — V0.3.2

- `python3 -m compileall`: PASS.
- Fixture SAME-SKELETON:
  - 3 SMB files;
  - `root.smb` e `root_attack.smb` têm bytes/payload diferentes;
  - ambos têm exatamente o mesmo bone hierarchy + bind pose;
  - índice agrupou os 3 arquivos em 2 skeleton signatures;
  - structural gate aceitou o skeleton compartilhado e rejeitou o outro;
  - resultado `UNIQUE_SKELETON`; PASS.
- Registry reload do fixture:
  - `binding_mode=ROOT_SKELETON_BOUND`;
  - `binding_source=SKELETON_SIGNATURE_UNIQUE`;
  - aliases SMB preservados; PASS.
- Fixture AMBIGUOUS-SKELETON:
  - structural gate aceita duas skeleton signatures realmente diferentes;
  - resultado `AMBIGUOUS_SKELETON_MATCH`, exit 4, sem promoção; PASS.
- Runtime/client real não está disponível neste ambiente. O scan de
  `char/tmABCD/CmfbB02.smd` no PC do projeto é o gate real desta versão.
