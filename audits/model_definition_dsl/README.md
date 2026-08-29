# Model Definition DSL Audit

## Context

O parser legado de modelos/motions utiliza keywords coreanas CP949 como
identificadores estruturais.

Esses valores não são localization. Eles controlam comportamento do parser.

Objetivo futuro:

- preservar compatibilidade com definições antigas;
- introduzir aliases ASCII/ingleses;
- remover dependência de idioma da lógica;
- não modificar arquivos Language;
- não converter todo o source para outro encoding.

## szIniCommand

| ID | Legacy Korean | Proposed ASCII |
|---:|---|---|
| 0 | *동작파일 | *MOTION_FILE |
| 1 | *동작모음 | *MOTION_SET |
| 2 | *모양파일 | *MODEL_FILE |
| 3 | *모양이름 | *MODEL_NAME |
| 4 | *파일연결 | *FILE_LINK |
| 5 | *보조동작파일 | *AUX_MOTION_FILE |
| 6 | *정밀모양 | *DETAIL_MODEL |
| 7 | *보통모양 | *DEFAULT_MODEL |
| 8 | *저질모양 | *LOW_MODEL |
| 9 | *XX걷는동작 | UNRESOLVED_LEGACY |
| 10 | *XX서있기동작 | UNRESOLVED_LEGACY |
| 11 | *착용무기 | *EQUIPPED_WEAPON |
| 12 | *적용직업 | *APPLICABLE_JOB |
| 13 | *해당위치 | *LOCATION |
| 14 | *적용기술 | *APPLICABLE_SKILL |
| 15 | *표정파일연결 | *FACE_FILE_LINK |
| 16 | *표정파일 | *FACE_FILE |
| 17 | *표정모음 | *FACE_SET |

## Special values

- 모두 -> ALL
- 없음 -> NONE
- 마을 -> TOWN
- 필드 -> FIELD

## Motion tokens

- 반복 -> REPEAT
- 동작 -> MOTION
- 뛰는 -> RUN
- *서있기 -> STAND
- *정지 -> STOP/IDLE
- *걷는 -> WALK
- 공격 -> ATTACK
- 죽기 -> DIE
- 죽는 -> DEAD
- 타격 -> HIT
- 데미지 -> DAMAGE
- 막기 -> BLOCK
- 울기 -> unresolved semantic ("CRY", variable is szWarpWord)
- 워프 -> WARP
- 가끔 -> SOMETIMES
- 떨어 -> FALL
- 큰착지 -> LARGE_LANDING
- 작은착지 -> SMALL_LANDING
- 다시시작 -> RESTART
- 먹기 -> EAT
- 기술 -> SKILL
- 만세 -> CHEER

## Safety rule

Nenhuma tradução desta tabela deve ser aplicada automaticamente ao source.

Primeiro deve ser provado quais arquivos textuais utilizam cada token e se
existem caches .inx envolvidos.
