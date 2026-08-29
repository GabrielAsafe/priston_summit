# Estado dos Bugs

## Resolvidos

* Cursor/asset do mouse.
* F11 crash.
* Mobs não spawnando.
* NPCs com modelo repetido.
* Fundo branco do chat.
* Céu preto.
* Delete de personagem.
* MessageBoxes/debug na inicialização.
* Backend de clã ASP → PHP.

## Parcial

* Logs no diretório do jogo — cliente quase resolvido; servidor ainda pendente.
* Alguns merchants não abrem. - resolvido mas ainda faltam os items, que está relacionado aos mobs só droparem dinheiro, que estava relacionado ao encoding, 

* Coreano/encoding corrompido no source.
## Abertos

* Mestre de classe oferecendo missão antes do nível — confirmar se é bug.


* quando crio o personagem ele inicia na ultima tier e depois de relogar ele volta para tier 1 como deve ser
* A imagem de quest não aparece. pode ser parecido com um bug de imagem em um buff, mas não tenho texto nas texboxes de item nem de quests. 
*quando resolvemos os npcs a instrutora aline desapareceu, mas antes ela estava lá. no lugar dela apareceu o enzo restaurado de items
* no loging a quantidade de caracteres cresce mas o tamanho da textbox é fixed então o texto some mas acho que continua a escrever
* sistema de quest bugado
* Nomes de personagens corrompidos.
* Assets/pastas duplicados por diferença de maiúsculas/minúsculas.
* GUI após resize.
* IPs/portas via configuração.
* Mostrar IP/porta no startup do servidor.
* Migrar o source para inglês futuramente.
* provavelmente tem um bug no spawn inicial do personagem pois independente da tripo eles abrem em ricaten e os de magos deviam nascer na outra

## Chat

Causa identificada no `sinInterFace.cpp`:

```text
BASE:    ChatDefault-1.tga
CURRENT: ChatDefault-1.png
```

Depois de normalizar encoding e finais de linha, restou apenas **1 alteração semântica** relacionada ao asset do chat.
