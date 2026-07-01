# Summit Tale - Site de MMORPG

## 🎨 Paleta de Cores

- **Primária**: #ff6b35 (Laranja vibrante)
- **Secundária**: #1a1a2e (Azul escuro)
- **Accent**: #ffd700 (Dourado)
- **Texto Claro**: #ffffff
- **Texto Escuro**: #333333

## 🔧 Como Editar

### Alterando Textos

1. **Título do Jogo**: Edite a classe `.game-title` no HTML
2. **Slogan**: Modifique o conteúdo da classe `.hero-slogan`
3. **Descrições**: Altere os textos nas seções correspondentes

### Modificando Cores

No arquivo `style.css`, altere as variáveis CSS no `:root`:

```css
:root {
    --primary-color: #ff6b35;    /* Cor principal */
    --secondary-color: #1a1a2e;  /* Cor secundária */
    --accent-color: #ffd700;     /* Cor de destaque */
}
```

### Trocando Imagens

1. Substitua os arquivos de imagem mantendo os mesmos nomes
2. Ou altere os caminhos no CSS:
   - `hero-bg.jpg` - Fundo da seção principal
   - `world-bg.jpg` - Fundo da seção mundo
   - `features-bg.jpg` - Fundo das características
   - `character.jpg` - Imagem do personagem



### Modificando Efeitos

- **Partículas**: Ajuste `particleCount` no script.js
- **Velocidade de animações**: Modifique as durações no CSS
- **Efeitos de hover**: Personalize as transições nas classes correspondentes


