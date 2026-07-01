# Summit Documentation Package

Copiar para a raiz do projeto `summit/`:

```bash
cp -r docs config tools "/home/gabriel/Área de Trabalho/summit/"
cd "/home/gabriel/Área de Trabalho/summit"
cp config/summit.local.env.example config/summit.local.env
bash tools/generate-sql-reg.sh config/summit.local.env Servidor/sql.reg
```
