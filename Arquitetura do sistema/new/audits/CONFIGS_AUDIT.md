# Audit — `/Servidor/Configs`

## Escopo

Arquivos recebidos/analisados: `ExpManager.ini`, `Sistemas.ini`, `hotuk.ini`, `Welcome.ini`. Este relatório deliberadamente não reproduz valores secretos.

## `ExpManager.ini`

**Status:** runtime ativo / candidato forte a source canonical.

Consumidores encontrados no server/runtime carregam parâmetros de EXP e party. A configuração foi materializada no bundle em `Content/canonical/gameplay_balance/experience.json` e existe um materializador de volta para INI para compatibilidade.

Decisão: JSON canonical vira autoria; INI passa a ser artefato de compatibilidade até o runtime consumir o pack diretamente.

## `Sistemas.ini`

**Status:** runtime ativo, mas arquiteturalmente misturado.

Contém ao mesmo tempo:

- gameplay/rates/bonus;
- flags mutáveis de operação/evento;
- configuração de infraestrutura;
- credenciais de acesso a dados.

Decisão: não migrar o arquivo inteiro para um pack compartilhável.

Separação alvo:

```text
gameplay estável -> Content/canonical + gameplay_config pack
runtime mutável   -> SQLite/admin settings server-only
infra/secrets     -> env/secret store/server-only
```

A migração deve também retirar credenciais do arquivo versionado/deploy compartilhado e rotacioná-las operacionalmente quando a equipe fizer a mudança.

## `hotuk.ini`

**Status:** runtime ativo / server-only.

`smConfigDecode("Configs//hotuk.ini")` mostra que ele participa de bootstrap/configuração legada. Mistura identidade/topologia e algumas flags relacionadas ao jogo.

Decisão: manter server-only; extrair somente parâmetros claramente de gameplay para canonical. Não levar endpoint/topologia/segredo ao cliente.

## `Welcome.ini`

**Status:** nenhum consumidor relevante encontrado na source fornecida.

Isso não prova que é descartável. Antes de remover:

1. file-open trace em staging;
2. boot/login/event smoke;
3. confirmar que nenhuma ferramenta externa lê o arquivo.

## Definition of done

- nenhum segredo em Content packs;
- EXP/party authored em canonical;
- parâmetros de gameplay restantes catalogados e classificados;
- mutable ops não exige rebuild de pack;
- arquivos sem consumidor só são removidos após trace real.
