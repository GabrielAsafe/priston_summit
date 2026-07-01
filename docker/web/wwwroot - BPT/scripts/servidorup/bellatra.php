<?php
// Configurações básicas de erro
error_reporting(E_ALL);
ini_set('display_errors', 1);

// Conexão com o banco de dados
$server = 'PTL\SQLEXPRESS';
$database = 'ClanDB';
$username = 'sa';
$password = '#$pdl32xq';
$connectionString = "Driver={ODBC Driver 17 for SQL Server};Server=$server;Database=$database;";

$conn = odbc_connect($connectionString, $username, $password);
if (!$conn) {
    die("<div style='color:white;background:red;padding:2px;font-size:9px;'>Erro na conexão</div>");
}

// Consulta SQL para obter o clã com maior SOD
$sql = "SELECT TOP 1 
           [MIconCnt] AS Icone,
           [ClanName] AS NomeClan, 
           [ClanZang] AS Lider, 
           [Cpoint] AS Pontos 
        FROM [CL] 
        ORDER BY [Cpoint] DESC";

$result = odbc_exec($conn, $sql);
if (!$result) {
    die("<div style='color:white;background:red;padding:2px;font-size:9px;'>Erro na consulta</div>");
}

$clan = odbc_fetch_array($result);
if (!$clan) {
    die("<div style='color:white;text-align:center;padding:10px;font-size:10px;'>Nenhum clã encontrado</div>");
}
?>
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Clã Líder</title>
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
            background: rgba(0, 0, 0, 0.5);
            border: 1px solid #8B0000;
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
        .clan-leader {
            color: #FFD700;
            font-size: 9px;
            margin-bottom: 2px;
        }
        .clan-points {
            color: #00FF00;
            font-weight: bold;
            font-size: 10px;
        }
    </style>
</head>
<body>
    <div class="clan-container">
        <?php
        $icon_path = "Clancontent/{$clan['Icone']}.bmp";
        echo "<img src='$icon_path' class='clan-icon' alt='Ícone do Clã' onerror=\"this.style.display='none'\">";
        echo "<div class='clan-name' title='{$clan['NomeClan']}'>" . htmlspecialchars($clan['NomeClan']) . "</div>";
        echo "<div class='clan-leader'>Líder: " . htmlspecialchars($clan['Lider']) . "</div>";
        echo "<div class='clan-points'>SOD: " . number_format($clan['Pontos']) . "</div>";
        ?>
    </div>
</body>
</html>
<?php
odbc_close($conn);
?>