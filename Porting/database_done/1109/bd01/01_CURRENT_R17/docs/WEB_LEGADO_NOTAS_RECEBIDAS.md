Existe um sistema web legado que eu também removeria no seu servidor local

Essa foi outra descoberta relevante.

TJBOY/isaocheck/isaocheck.cpp contém bastante comunicação HTTP legada.

Inclusive:

sprintf(
    szWriteBuffer,
    "http://%s/login.php?UserID=%s&PassWord=%s",
    hostName,
    id,
    pass
);

e:

"http://%s/InsertUser.php?UserID=%s&PassWord=%s&UserName=%s"

Ou seja: existem rotinas antigas capazes de colocar usuário e senha na URL HTTP.

Encontrei também endereços legados:

211.39.144.157
211.39.144.158
74.122.192.107

e:

http://211.39.144.158/NPCSound/

Essas strings também estão no Game.exe atual, portanto esse código foi compilado.

Porém há uma distinção importante: não encontrei chamada atual para o ISAOCHECK::IsaoCheck() que transmite a senha dessa maneira.

Então eu classificaria assim:

código perigoso legado: SIM
exfiltração ativa de senha por esse caminho: NÃO CONFIRMADA

Parte dessa infraestrutura ainda é usada por recursos antigos de:

clan
notices
clan images
SoD ranking
NPC sounds

Para um servidor puramente local, eu removeria ou colocaria toda essa camada atrás de uma opção desabilitada por padrão.
