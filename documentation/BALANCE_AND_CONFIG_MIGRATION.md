# Balanceamento, EXP, drops, gold e `/Servidor/Configs`

## Planilha

`spreadsheets/Summit_Balanceamento_Atual_com_Nomes_Itens.xlsx` contém:

- 98 monstros analisados;
- 7.003 entradas de drop expandidas;
- nomes/canonical IDs/assets dos itens quando resolvidos;
- curva cumulativa de nível 1..161;
- party rates;
- chance efetiva dos drops.

## Loot

`FallItem` usa peso relativo, não porcentagem direta.

A chance de uma entrada em uma roll normal é aproximadamente:

`peso_efetivo / soma_dos_pesos_do_monstro`

Quando uma linha possui vários códigos, o parser divide o peso por quantidade usando divisão inteira; pode sobrar peso sem item associado.

`ExtraFallItem` é um mecanismo diferente, em base 10.000 e independente da roll normal.

`FallItemMax` controla a quantidade base de tentativas/rolls.

## EXP

Foi canonicalizado `Configs/ExpManager.ini` em:

`Content/canonical/gameplay_balance/experience.json`.

Ferramenta:

```bash
python tools/materialize_experience_ini.py \
  Content/canonical/gameplay_balance/experience.json \
  Servidor/Configs/ExpManager.ini
```

Foi incluído patch para `srGetTotalExp`: a função recebe INT64 mas usava `int result`, podendo overflowar antes do retorno. O patch usa INT64 e percentuais inteiros determinísticos.

## Party

O runtime atual carrega rates 2..6 do INI, mas 7/8 permanecem hardcoded. `configure_party_2_to_8.patch` passa a aceitar:

```ini
7 Killer=22
7 Membro=21
8 Killer=20
8 Membro=20
```

mantendo fallback para a chave antiga `N Membros`.

## `/Servidor/Configs`

### `ExpManager.ini`

Gameplay/balance puro. Deve ser canonical e materializado/compilado.

### `Sistemas.ini`

Mistura gameplay, estado mutável e infraestrutura/segredos. Não colocar o arquivo inteiro em `gameplay_config.sgcf`.

Separar em:

- canonical gameplay: rates, bonus values, event parameters estáveis;
- runtime mutable settings: liga/desliga evento, rates temporários, maintenance;
- secrets/server infra: DB credentials/topology, fora de packs compartilhados.

### `hotuk.ini`

Mistura bootstrap/topologia e alguns controles de jogo. Manter server-only e migrar somente os campos de gameplay comprovadamente imutáveis.

### `Welcome.ini`

Não foi encontrado consumidor relevante na análise. Tratar como legado até um runtime trace confirmar uso; não apagar apenas por ausência de grep.

## Processo recomendado de balance

1. corrigir overflow e externalizar todos party rates;
2. definir tempo alvo por faixa de nível;
3. usar `EXP até próximo / EXP efetiva por kill` para estimar kills;
4. medir kill time real por classe/faixa;
5. balancear XP/h, não apenas XP por mob;
6. calcular gold esperado por kill com peso real;
7. definir budget de drops por hora;
8. validar bosses/eventos separadamente;
9. simular party solo/2/4/6/8 antes de publicar.
