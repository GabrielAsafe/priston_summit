# Fix 015 - Character list bounds

## Estado

PENDENTE DE TESTE.

## Problema

TRANS_USERCHAR_INFO usa:

    sPLAY_CHAR_MAX = 11

O cliente possuia:

    DownUserCharacterInfo[MAX_DOWN_USER_LIST_INFO]
    MAX_DOWN_USER_LIST_INFO = 10

e a tela de selecao possui:

    UserCharacter[CHARACTER_USER_MAX]
    CHARACTER_USER_MAX = 9

HoRecvMessage copiava diretamente:

    ServerUserCharacterInfo->PlayUserCount

para DownUserCharacterInfo[] e depois atribuía o mesmo valor a
DownUserCharacterCount.

Isso permitia:

1. escrita alem de DownUserCharacterInfo[] caso count > 10;
2. DownUserCharacterCount > 9 alimentar loops que acessam
   UserCharacter[index], cujo limite e 9.

## Alteracao

Arquivo alterado:

    SourceCode/SrcGame/src/HoBaram/HoLinkHeader.cpp

PlayUserCount agora e normalizado para:

    0 .. CHARACTER_USER_MAX

antes de qualquer copia.

O mesmo valor normalizado e gravado em DownUserCharacterCount.

## Fora do escopo

Nao foi alterado:

- HoLogin::AddJobCharacter();
- CHARACTER_JOB_MAX;
- protocolo TRANS_USERCHAR_INFO;
- servidor;
- quantidade maxima armazenada no servidor.

O AddJobCharacter possui um problema de bounds independente e deve ser
tratado em fix separado.

## Observacao sobre personagens duplicados

Na reproducao atual, o log mostrou PlayUserCount=2.

Portanto este patch corrige um bug real de bounds, mas o sintoma de
personagens duplicados so deve ser considerado resolvido depois do teste
funcional.

## Teste

1. compilar/deploy cliente;
2. abrir cliente;
3. logar na conta;
4. confirmar que aparecem somente os personagens reais;
5. selecionar cada personagem;
6. entrar no mapa;
7. voltar para selecao/login quando possivel;
8. confirmar ausencia de crash ou slot fantasma.

Somente depois do teste positivo o fix deve ser commitado.
