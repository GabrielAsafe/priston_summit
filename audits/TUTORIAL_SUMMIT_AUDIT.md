# Tutorial de utilização — Summit Asset Audit Toolkit

Este tutorial assume que o terminal está em:

```bash
cd "/home/gabriel/Área de Trabalho/summit/audits"
```

Todos os exemplos abaixo partem desse diretório.

---

# 1. Organização recomendada

Estrutura sugerida:

```text
summit/
├── Summit Tale Client Full/
│   └── cliente/
├── Servidor/
├── SourceCode/
├── bugfixes/
└── audits/
    ├── summit_inventory_audit.py
    ├── summit_phase2_assets.py
    ├── summit_quarantine.py
    ├── quarantine_client_candidates.csv
    ├── quarantine_server_candidates.csv
    ├── summit_inventory_audit/
    ├── summit_phase2/
    └── quarantine/
```

---

# 2. Fase 1 — inventário geral

Execute:

```bash
cd "/home/gabriel/Área de Trabalho/summit/audits"

python3 summit_inventory_audit.py \
  --root "/home/gabriel/Área de Trabalho/summit" \
  --out "./summit_inventory_audit"
```

Saída esperada:

```text
1/7 inventariando cliente...
2/7 inventariando servidor...
3/7 procurando duplicados exatos...
4/7 reconstruindo conjuntos compilados e grafo de includes...
5/7 indexando referências de assets...
6/7 cruzando assets cliente x referências...
7/7 gravando relatórios...
```

## O que revisar primeiro

```text
summit_inventory_audit/RESUMO.txt
summit_inventory_audit/catalog_diretorios_cliente.csv
summit_inventory_audit/catalog_diretorios_servidor.csv
summit_inventory_audit/cliente_assets_soltos_candidatos.csv
summit_inventory_audit/referencias_codigo_ausentes_no_cliente.csv
summit_inventory_audit/shared2_usage.csv
```

## Atenção

`cliente_assets_soltos_candidatos.csv` é uma fila de investigação, não uma lista de exclusão.

---

# 3. Fase 2 — dependências de assets

Execute:

```bash
cd "/home/gabriel/Área de Trabalho/summit/audits"

python3 summit_phase2_assets.py \
  --root "/home/gabriel/Área de Trabalho/summit" \
  --phase1-dir "./summit_inventory_audit" \
  --out "./summit_phase2"
```

Opcionalmente é possível limitar o tamanho máximo de arquivo analisado:

```bash
python3 summit_phase2_assets.py \
  --root "/home/gabriel/Área de Trabalho/summit" \
  --phase1-dir "./summit_inventory_audit" \
  --out "./summit_phase2" \
  --max-file-mb 32
```

## Relatórios principais

```text
summit_phase2/RESUMO_FASE2.txt
summit_phase2/phase2_asset_status_char_field.csv
summit_phase2/phase2_edges.csv
summit_phase2/phase2_orphans_high_confidence.csv
summit_phase2/phase2_broken_references.csv
summit_phase2/phase2_ambiguous_references.csv
```

## Como interpretar

Não promover um arquivo para quarentena apenas porque aparece em:

```text
phase2_orphans_high_confidence.csv
```

Primeiro confirmar:

1. se existe um loader dinâmico;
2. se uma extensão é substituída em runtime;
3. se o basename é construído;
4. se existe índice/configuração intermediária;
5. se o asset é uma dependência de um modelo.

### Regra especial descoberta

Para `Field`, uma referência:

```text
foo.ASE
```

pode carregar:

```text
foo.smd
```

no mesmo diretório.

Portanto qualquer scanner futuro deve resolver essa equivalência antes de classificar `foo.smd` como órfão.

---

# 4. Criando uma lista de quarentena

O arquivo CSV mínimo possui:

```csv
side,relpath,confidence,reason,enabled
client,Field/exemplo/arquivo.tga,HIGH,sem referência ativa,1
```

Valores válidos de `side`:

```text
client
server
```

Exemplo de cliente:

```csv
side,relpath,confidence,reason,enabled
client,Field/old_forest,VERY_HIGH,árvore legacy sem referência ativa,1
client,Field/old_dungeon,VERY_HIGH,árvore legacy sem referência ativa,1
```

Exemplo de servidor:

```csv
side,relpath,confidence,reason,enabled
server,Logs/old.log,HIGH,log histórico,1
```

A separação é automática:

```text
audits/quarantine/cliente/
audits/quarantine/servidor/
```

---

# 5. Sempre começar com `plan`

Nunca iniciar diretamente com `apply`.

Exemplo:

```bash
cd "/home/gabriel/Área de Trabalho/summit/audits"

python3 summit_quarantine.py plan \
  --run-id minha_auditoria_20260825
```

O `plan` mostra:

- origem;
- destino;
- confiança;
- motivo;
- quantidade de arquivos;
- tamanho total.

Ele **não move nada**.

---

# 6. Aplicar a quarentena

Depois de conferir o plano:

```bash
cd "/home/gabriel/Área de Trabalho/summit/audits"

python3 summit_quarantine.py apply \
  --run-id minha_auditoria_20260825
```

Nunca reutilizar um `run-id` que já gerou manifesto.

O script cria:

```text
audits/quarantine/manifests/minha_auditoria_20260825.csv
audits/quarantine/manifests/minha_auditoria_20260825.json
```

O JSON também registra erros parciais.

---

# 7. Usar um CSV específico

Para uma rodada pequena:

```bash
python3 summit_quarantine.py plan \
  --run-id teste_um_arquivo \
  --candidates arquivo_especifico.csv
```

Depois:

```bash
python3 summit_quarantine.py apply \
  --run-id teste_um_arquivo \
  --candidates arquivo_especifico.csv
```

Isto é útil quando uma rodada maior teve um único erro.

---

# 8. Calcular hash de arquivos movidos

Para arquivos individuais:

```bash
python3 summit_quarantine.py apply \
  --run-id lote_com_hash \
  --candidates candidatos.csv \
  --hash
```

O SHA256 é gravado no manifesto para arquivos.

Para diretórios, o script registra quantidade de arquivos e volume, não um hash global da árvore.

---

# 9. Ver estado das quarentenas

Execute:

```bash
cd "/home/gabriel/Área de Trabalho/summit/audits"

python3 summit_quarantine.py status
```

Exemplo conceitual:

```text
Quarentena: .../audits/quarantine
Manifestos: 2
  field_legacy_20260825: 24 movidos / 161.20 MiB
  field_world_ani06_20260825: 1 movido / ...
```

---

# 10. Restaurar uma rodada

Se o jogo quebrar após uma quarentena:

```bash
cd "/home/gabriel/Área de Trabalho/summit/audits"

python3 summit_quarantine.py restore \
  --run-id field_legacy_20260825
```

A restauração:

- lê o manifesto;
- percorre os itens em ordem reversa;
- não sobrescreve um arquivo que já tenha sido recriado no caminho original;
- gera um log `.restore.json`.

Se apenas outro lote causou o problema, restaurar somente esse `run-id`.

---

# 11. Troubleshooting de `Permission denied`

Um caso real encontrado foi:

```text
Field/Ricarten
```

com permissão:

```text
dr-xr-xr-x
```

O arquivo tinha `0644`, mas o diretório pai não permitia escrita.

Para diagnosticar:

```bash
cd "/home/gabriel/Área de Trabalho/summit/audits"

FILE="../Summit Tale Client Full/cliente/Field/Ricarten/world_ani06.ASE"

ls -l "$FILE"
ls -ld "$(dirname "$FILE")"
stat "$FILE"
namei -l "$FILE"
lsattr "$FILE" 2>/dev/null || true
getfacl "$FILE" 2>/dev/null || true
```

Se o diretório pertence ao usuário e apenas perdeu escrita:

```bash
chmod u+w "../Summit Tale Client Full/cliente/Field/Ricarten"
```

Executar a quarentena.

Depois restaurar a permissão original:

```bash
chmod 555 "../Summit Tale Client Full/cliente/Field/Ricarten"
```

Evitar:

```text
chmod 777
```

e evitar `sudo` quando não é necessário.

---

# 12. Protocolo de teste depois de `apply`

Para assets de mapa/cenário, testar pelo menos:

```text
[ ] cliente inicia
[ ] login funciona
[ ] personagem entra no mundo
[ ] Ricarten
[ ] Forest
[ ] Dungeon
[ ] Cave
[ ] Forever Fall
[ ] teleporte
[ ] mapa-múndi
[ ] título/nome de área
[ ] texturas do cenário
[ ] objetos animados
[ ] entrar/sair novamente das áreas
[ ] fechar e abrir o cliente novamente
```

Também verificar logs procurando:

```text
file not found
cannot open
texture
material
ASE
SMD
SMB
missing
error
```

Um único teste positivo não transforma automaticamente o lote em remoção definitiva.

---

# 13. Estado atual da primeira rodada

Já foi executada uma quarentena envolvendo:

```text
Field/old_forest
Field/old_dungeon
mapas/títulos legacy relacionados
Field/cave/Dcave_anii14.ASE
Field/forever-fall/3ani-08.ASE
Field/Ricarten/world_ani06.ASE
```

Resultado funcional já observado:

```text
cliente iniciou
nenhum crash imediato
Ricarten apareceu normalmente
```

Estado:

```text
QUARANTINE_TESTING
```

Ainda testar as demais áreas antes de promover para:

```text
REMOVABLE_CONFIRMED
```

---

# 14. Como remover definitivamente no futuro

A recomendação é **não possuir um comando automático de purge nesta fase**.

Quando um lote estiver:

```text
REMOVABLE_CONFIRMED
```

procedimento recomendado:

1. manter a quarentena por um período;
2. fazer backup do manifesto;
3. registrar no overview/CHANGELOG o que foi removido;
4. só então apagar a pasta correspondente dentro de `audits/quarantine/...`;
5. manter os manifestos e relatórios históricos.

Assim existe rastreabilidade mesmo depois da remoção física.

---

# 15. Próxima fase: `char/`

A próxima investigação deve começar por `char/`.

Objetivos:

```text
identificar NPCs
identificar monstros
identificar personagens/skins
mapear INI
mapear SMD/SMB
mapear texturas
mapear animações
separar temp/copy
achar duplicados
achar famílias antigas
```

Antes de declarar qualquer modelo órfão, localizar no código:

```text
LoadOftenMeshPattern
LoadMesh
LoadPattern
SMD
SMB
INI
INX
monster
npc
character
```

e reconstruir o comportamento dos loaders.

---

# 16. Próxima fase: servidor

Fazer uma auditoria separada.

Principais grupos já conhecidos para revisão:

```text
Crash.dmp
wine.log
logs históricos
PDBs
LogItemData
backups
userdata antigo
arquivos temporários
```

Separar sempre:

```text
lixo operacional
backup
debug
runtime obrigatório
dados persistentes
```

Nunca aplicar a mesma política de asset do cliente diretamente ao servidor.

---

# 17. Próxima fase: código legado

### `Shared2`

Executar:

```bash
cd "/home/gabriel/Área de Trabalho/summit/audits"

diff -ru \
  "../SourceCode/Shared" \
  "../SourceCode/Shared2" \
  > shared_vs_shared2.diff || true
```

Depois revisar os três headers e builds alternativos.

### Loader ASE

Corrigir o bug documentado em:

```text
bugfixes/021_ase_path_parser_audit/
```

Depois recompilar e testar antes de tocar em assets que dependam da conversão ASE → SMD.

---

# 18. Reexecutar após cada grande alteração

Depois de um conjunto relevante de mudanças:

```bash
cd "/home/gabriel/Área de Trabalho/summit/audits"

python3 summit_inventory_audit.py \
  --root "/home/gabriel/Área de Trabalho/summit" \
  --out "./summit_inventory_audit_next"
```

Depois:

```bash
python3 summit_phase2_assets.py \
  --root "/home/gabriel/Área de Trabalho/summit" \
  --phase1-dir "./summit_inventory_audit_next" \
  --out "./summit_phase2_next"
```

Usar nomes diferentes ajuda a comparar antes/depois.

---

# 19. Checklist para retomar o projeto no futuro

Quando voltar à auditoria:

```text
[ ] ler README_SUMMIT_AUDIT.md
[ ] executar summit_quarantine.py status
[ ] verificar lotes QUARANTINE_TESTING
[ ] concluir smoke tests pendentes
[ ] revisar bugfixes pendentes
[ ] reexecutar inventário se a árvore mudou
[ ] escolher apenas uma família de assets por vez
[ ] analisar loader antes de quarentena em massa
[ ] criar run-id novo
[ ] plan
[ ] apply
[ ] testar
[ ] restaurar se houver regressão
[ ] documentar decisão
```

---

# 20. Filosofia da ferramenta

A ferramenta não tenta decidir sozinha o que é lixo.

Ela serve para transformar:

```text
"acho que isso não usa"
```

em:

```text
"não há referência direta,
não há dependência indireta conhecida,
o loader não possui fallback,
foi colocado em quarentena,
o jogo foi testado,
não houve regressão."
```

Essa sequência é a diferença entre uma limpeza controlada e uma remoção arriscada.
