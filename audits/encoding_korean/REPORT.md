# Encoding Audit Report

Auditoria somente-leitura. Nenhum arquivo foi convertido ou modificado.

## Resumo

- Arquivos alvo analisados: 16
- Arquivos fonte analisados: 8
- Alvos contendo U+FFFD: 16
- Total de U+FFFD nos alvos: 17834
- Candidatos com fonte equivalente limpa e CP949 válido: 16

## Regra de interpretação

- `U+FFFD > 0`: a perda já está gravada no arquivo alvo.
- `CP949 válido + U+FFFD=0` numa fonte antiga: forte candidato, não prova sozinho equivalência semântica.
- `latin-1` não é usado como prova porque aceita praticamente qualquer byte.
- Nenhuma substituição automática deve ser feita a partir deste relatório.

## Melhores candidatos de recuperação

| Target | FFFD | Fonte | CP949 | Estrutura igual |
|---|---:|---|:---:|:---:|
| `Language/Korean/k_ServerMsg.h` | 5275 | `pstale` | sim | não |
| `Language/Korean/k_ServerMsg.h` | 5275 | `pstale` | sim | não |
| `Language/Korean/k_clanText.h` | 1971 | `pstale` | sim | não |
| `Language/Korean/k_clanText.h` | 1971 | `pstale` | sim | não |
| `Language/Korean/k_QuestMsg.h` | 886 | `pstale` | sim | não |
| `Language/Korean/k_QuestMsg.h` | 886 | `pstale` | sim | não |
| `Language/Korean/k_TextMessage.h` | 458 | `pstale` | sim | não |
| `Language/Korean/k_TextMessage.h` | 458 | `pstale` | sim | não |
| `Language/Korean/k_JobCode.h` | 189 | `pstale` | sim | não |
| `Language/Korean/k_JobCode.h` | 189 | `pstale` | sim | não |
| `Language/Korean/k_PartyMsg.h` | 76 | `pstale` | sim | sim |
| `Language/Korean/k_PartyMsg.h` | 76 | `pstale` | sim | sim |
| `Language/Korean/K_VolText.h` | 50 | `pstale` | sim | sim |
| `Language/Korean/K_VolText.h` | 50 | `pstale` | sim | sim |
| `Language/Korean/K_TalkText.h` | 12 | `pstale` | sim | sim |
| `Language/Korean/K_TalkText.h` | 12 | `pstale` | sim | sim |

## Próxima ação recomendada

Selecionar um único candidato, comparar estrutura/identificadores/índices, extrair a fonte em memória com CP949 e criar um bugfix BEFORE/AFTER. Não substituir arquivos inteiros sem provar compatibilidade.
