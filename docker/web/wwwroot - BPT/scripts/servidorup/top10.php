<?php
// Configurações do banco de dados
$server = 'PTL\SQLEXPRESS';
$database = 'ServerDB';
$username = 'sa';
$password = '#$pdl32xq';
$websiteUrl = 'https://summittale.com.br/';
$imagesPath = '/imagens';

// Conexão ODBC
$connectionString = "Driver={ODBC Driver 17 for SQL Server};Server=$server;Database=$database;";
$conn = odbc_connect($connectionString, $username, $password);

if (!$conn) {
    die("<div style='color:white;font-size:9px;padding:2px;'>Erro de conexão: " . htmlspecialchars(odbc_errormsg()) . "</div>");
}

// Consulta SQL para TOP 10
$sql = "SELECT TOP 10 [Name], [Class], [Lvl] FROM [PVP] ORDER BY [Lvl] DESC";
$result = odbc_exec($conn, $sql);

if (!$result) {
    die("<div style='color:white;font-size:9px;padding:2px;'>Erro na consulta: " . htmlspecialchars(odbc_errormsg()) . "</div>");
}
?>
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <title>TOP 10</title>
    <style>
        body {
            margin: 0;
            padding: 0;
            background: transparent;
            font-family: Verdana, Arial Narrow, sans-serif;
            font-size: 9px;
            line-height: 1.1;
            color: white;
            width: 155px;
            overflow: hidden;
        }
        .top10-table {
            width: 100%;
            border-collapse: collapse;
            table-layout: fixed;
        }
        .top10-table th {
            background: #8B0000;
            color: #FFD700;
            padding: 2px 1px;
            text-align: center;
            font-weight: bold;
            font-size: 8px;
            height: 14px;
        }
        .top10-table td {
            padding: 1px;
            border-bottom: 1px solid #333;
            text-align: center;
            vertical-align: middle;
            height: 14px;
            overflow: hidden;
        }
        .top10-table tr:nth-child(even) {
            background: rgba(139, 0, 0, 0.2);
        }
        .rank {
            width: 14px;
            font-weight: bold;
            color: #FFD700;
        }
        .player-name {
            text-align: left;
            padding-left: 2px;
            white-space: nowrap;
            overflow: hidden;
            text-overflow: ellipsis;
        }
        .class-icon {
            width: 12px;
            height: 12px;
            vertical-align: middle;
            margin-right: 1px;
        }
        .level-col {
            width: 25px;
        }
    </style>
</head>
<body>
    <table class="top10-table">
        <thead>
            <tr>
                <th class="rank">#</th>
                <th>Jogador</th>
                <th class="level-col">Lvl</th>
            </tr>
        </thead>
        <tbody>
            <?php
            $position = 1;
            while ($row = odbc_fetch_array($result)) {
                echo "<tr>
                        <td class='rank'>{$position}</td>
                        <td class='player-name'>
                            <img src='{$websiteUrl}{$imagesPath}/classes/{$row['Class']}.gif' 
                                 class='class-icon' alt='{$row['Class']}'>
                            ".htmlspecialchars($row['Name'])."
                        </td>
                        <td>{$row['Lvl']}</td>
                      </tr>";
                $position++;
            }
            
            // Completa até 10 linhas se necessário
            while ($position <= 10) {
                echo "<tr>
                        <td class='rank'>{$position}</td>
                        <td class='player-name'>-</td>
                        <td>0</td>
                      </tr>";
                $position++;
            }
            ?>
        </tbody>
    </table>
</body>
</html>
<?php
odbc_close($conn);