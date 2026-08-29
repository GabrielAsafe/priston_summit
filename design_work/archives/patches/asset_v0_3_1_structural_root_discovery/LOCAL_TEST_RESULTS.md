# Local test results — V0.3.1

- `python3 -m compileall`: PASS.
- Fixture UNIQUE: config-only SMD + 2 SMBs, fake structural gate aceita apenas 1 => `UNIQUE_PASS`; registry reload => `ROOT_SKELETON_BOUND` / `STRUCTURAL_GATE_UNIQUE`: PASS.
- Fixture AMBIGUOUS: fake structural gate aceita 2 SMBs de SHA diferente => `AMBIGUOUS_STRUCTURAL_MATCH`, exit 4, sem promoção: PASS.
- Runtime/client real não está disponível neste ambiente; o primeiro scan deve ser executado no PC do projeto.
