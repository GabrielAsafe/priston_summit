# Summit Tale — documentação organizada

Este repositório reorganiza o pacote histórico sem alterar o conteúdo dos ficheiros originais.

## Estrutura

- `docs/current/` — documentação numerada que funciona como referência operacional atual.
- `docs/guides/` — guias temáticos, incluindo PHP.
- `docs/reference/` — inventários, esquemas, diagramas e referências técnicas.
- `docs/history/` — documentação histórica, retomadas, relatórios, migrações e snapshots antigos.
- `artifacts/` — binários, PCAPs, dumps e resultados de análise; mantidos no pacote, mas fora do versionamento Git por padrão.
- `config/` — exemplos de configuração existentes no pacote original.
- `tools/` — ferramentas auxiliares existentes no pacote original.

## Integridade do conteúdo

Os ficheiros provenientes do pacote original foram apenas movidos: o conteúdo é preservado byte a byte. Isto inclui a documentação de credenciais/passwords e os valores nela existentes. O ficheiro `SOURCE_MAP.tsv` permite rastrear cada caminho original para o novo caminho.

## Git

A documentação textual, incluindo a documentação histórica, deve ser versionada. `artifacts/` fica ignorado por padrão porque contém binários, capturas e dumps pesados/derivados. Se esses artefatos precisarem de histórico próprio, use Git LFS ou armazenamento de releases em vez de misturá-los ao histórico normal.
