<?php
// Configuração obrigatória do timezone
date_default_timezone_set('America/Sao_Paulo');

// Configurações de conexão
$server = 'PTL\SQLEXPRESS';
$database = 'ClanDB';
$username = 'sa';
$password = '#$pdl32xq';
$connectionString = "Driver={ODBC Driver 17 for SQL Server};Server=$server;Database=$database;";

// Conexão com o banco de dados
$conn = odbc_connect($connectionString, $username, $password);
if (!$conn) {
    die("<div style='color:white;background:red;padding:5px;font-size:10px;text-align:center;'>
           ERRO DE CONEXÃO<br>
           Verifique:<br>
           - Servidor SQL está online<br>
           - Credenciais estão corretas<br>
           - Driver ODBC instalado
         </div>");
}

// Consulta para obter as informações do clã líder
$sql = "SELECT TOP 1 
           [MIconCnt] AS Icone,
           [ClanName] AS NomeClan, 
           [ClanZang] AS Lider, 
           [CWin] AS Vitorias
        FROM [CL] 
        WHERE LIDER_BC = 1";

$result = odbc_exec($conn, $sql);
if (!$result) {
    odbc_close($conn);
    die("<div style='color:white;background:red;padding:5px;font-size:10px;text-align:center;'>
           ERRO NA CONSULTA SQL<br>
           ".odbc_errormsg()."
         </div>");
}

$clan = odbc_fetch_array($result);
?>
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Clã Líder - Summit Tale</title>
    <style>
        body {
            margin: 0;
            padding: 0;
            background: transparent;
            font-family: Verdana, sans-serif;
            font-size: 10px;
            color: white;
            width: 154px;
            height: 70px;
            overflow: hidden;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        .clan-container {
            width: 100%;
            height: 100%;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            text-align: center;
            background: rgba(0, 0, 0, 0.6);
            border: 1px solid #8B0000;
            box-sizing: border-box;
        }
        .clan-icon {
            width: 24px;
            height: 24px;
            margin-bottom: 3px;
        }
        .clan-name {
            font-weight: bold;
            font-size: 11px;
            margin-bottom: 2px;
            white-space: nowrap;
            overflow: hidden;
            text-overflow: ellipsis;
            max-width: 140px;
        }
        .clan-info {
            color: #FFD700;
            font-size: 9px;
            margin: 1px 0;
        }
        .clan-victories {
            color: #00FF00;
            font-size: 9px;
            font-weight: bold;
        }
    </style>
</head>
<body>
    <div class="clan-container">
        <?php if (!$clan): ?>
            <div style="color:#aaa;">Sem clã líder</div>
        <?php else: ?>
            <?php
            $icon_path = "Clancontent/{$clan['Icone']}.bmp";
            echo "<img src='$icon_path' class='clan-icon' alt='Ícone do Clã' onerror=\"this.style.display='none'\">";
            echo "<div class='clan-name' title='{$clan['NomeClan']}'>".htmlspecialchars($clan['NomeClan'])."</div>";
            echo "<div class='clan-info'>Líder: ".htmlspecialchars($clan['Lider'])."</div>";
            echo "<div class='clan-victories'>Vitórias: ".$clan['Vitorias']."</div>";
            ?>
        <?php endif; ?>
    </div>
</body>
</html>
<?php
odbc_close($conn);
?>