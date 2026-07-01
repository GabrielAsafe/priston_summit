<?php
// ConfiguraÃƒÂ§ÃƒÂµes
$server = 'PTL\SQLEXPRESS';
$database = 'ServerDB';
$username = 'sa';
$password = '#$pdl32xq';
$websiteUrl = 'https://summittale.com.br/';
$imagesPath = '/imagens';

// ConexÃƒÂ£o ODBC
$connectionString = "Driver={ODBC Driver 17 for SQL Server};Server=$server;Database=$database;";
$conn = odbc_connect($connectionString, $username, $password);

if (!$conn) {
    die("<div style='font-family:Arial;max-width:800px;margin:50px auto;padding:20px;background:#fff3f3;border-radius:8px;border-left:4px solid #ff0000;'>
            <h2 style='color:#d32f2f;'>Erro de ConexÃƒÂ£o</h2>
            <p style='color:#5f2120;'>".odbc_errormsg()."</p>
            <div style='background:#fff;padding:15px;border-radius:4px;margin-top:15px;'>
                <h3 style='margin-top:0;'>InformaÃƒÂ§ÃƒÂµes TÃƒÂ©cnicas</h3>
                <p><strong>String de ConexÃƒÂ£o:</strong> ".htmlspecialchars($connectionString)."</p>
                <p><strong>UsuÃƒÂ¡rio:</strong> ".htmlspecialchars($username)."</p>
            </div>
        </div>");
}

// Consulta SQL
$sql = "SELECT TOP 100 [Name], [Class], [Lvl] FROM [PVP] ORDER BY [Lvl] DESC";
$result = odbc_exec($conn, $sql);

if (!$result) {
    die("<div style='font-family:Arial;max-width:800px;margin:50px auto;padding:20px;background:#fff3f3;border-radius:8px;border-left:4px solid #ff0000;'>
            <h2 style='color:#d32f2f;'>Erro na Consulta</h2>
            <p style='color:#5f2120;'>".odbc_errormsg()."</p>
        </div>");
}
?>
<!DOCTYPE html>
<html lang="pt-BR">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Ranking Summit Tale</title>
    <style>
        :root {
            --primary-color: #4a6bff;
            --secondary-color: #6c757d;
            --accent-color: #ff6b6b;
            --light-bg: #f8f9fa;
            --dark-bg: #343a40;
            --text-color: #212529;
            --text-light: #f8f9fa;
        }
        
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: var(--light-bg);
            color: var(--text-color);
            margin: 0;
            padding: 0;
            line-height: 1.6;
        }
        
        .container {
            max-width: 900px;
            margin: 40px auto;
            padding: 0 20px;
        }
        
        .ranking-card {
            background-color: white;
            border-radius: 10px;
            box-shadow: 0 4px 20px rgba(0, 0, 0, 0.08);
            overflow: hidden;
            margin-bottom: 30px;
        }
        
        .ranking-header {
            background: linear-gradient(135deg, var(--primary-color), #304ffe);
            color: white;
            padding: 20px;
            text-align: center;
        }
        
        .ranking-header h1 {
            margin: 0;
            font-size: 28px;
            font-weight: 600;
        }
        
        .ranking-header p {
            margin: 5px 0 0;
            opacity: 0.9;
        }
        
        .ranking-table {
            width: 100%;
            border-collapse: collapse;
        }
        
        .ranking-table th {
            background-color: #f1f3f9;
            color: var(--secondary-color);
            padding: 15px;
            text-align: left;
            font-weight: 500;
            text-transform: uppercase;
            font-size: 12px;
            letter-spacing: 0.5px;
        }
        
        .ranking-table td {
            padding: 15px;
            border-bottom: 1px solid #eee;
            vertical-align: middle;
        }
        
        .ranking-table tr:last-child td {
            border-bottom: none;
        }
        
        .ranking-table tr:hover td {
            background-color: #f8f9ff;
        }
        
        .rank {
            font-weight: 700;
            color: var(--primary-color);
            text-align: center;
            width: 50px;
        }
        
        .player-info {
            display: flex;
            align-items: center;
            gap: 12px;
        }
        
        .class-icon {
            width: 24px;
            height: 24px;
            object-fit: contain;
        }
        
        .player-name {
            font-weight: 500;
        }
        
        .player-level {
            font-weight: 700;
            color: var(--accent-color);
            text-align: right;
        }
        
        .badge {
            display: inline-block;
            padding: 3px 8px;
            border-radius: 4px;
            font-size: 12px;
            font-weight: 600;
            background-color: #e9ecef;
            color: var(--secondary-color);
        }
        
        .last-updated {
            text-align: right;
            font-size: 12px;
            color: var(--secondary-color);
            padding: 15px 20px;
            border-top: 1px solid #eee;
        }
        
        @media (max-width: 768px) {
            .container {
                padding: 0 15px;
            }
            
            .ranking-table th, 
            .ranking-table td {
                padding: 12px 10px;
            }
            
            .player-info {
                gap: 8px;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="ranking-card">
            <div class="ranking-header">
                <h1>Ranking Summit Tale</h1>
                <p>Top 100 melhores jogadores</p>
            </div>
            
            <table class="ranking-table">
                <thead>
                    <tr>
                        <th>#</th>
                        <th>Jogador</th>
                        <th>Level</th>
                    </tr>
                </thead>
                <tbody>
                    <?php
                    $position = 1;
                    while ($row = odbc_fetch_array($result)) {
                        echo "<tr>
                                <td class='rank'>{$position}</td>
                                <td>
                                    <div class='player-info'>
                                        <img src='{$websiteUrl}{$imagesPath}/classes/{$row['Class']}.gif' 
                                             alt='Classe {$row['Class']}' class='class-icon'>
                                        <span class='player-name'>".htmlspecialchars($row['Name'])."</span>
                                    </div>
                                </td>
                                <td class='player-level'>{$row['Lvl']}</td>
                              </tr>";
                        $position++;
                    }
                    ?>
                </tbody>
            </table>
            
            <div class="last-updated">
                Atualizado em <?= date('d/m/Y \ÃƒÂ \s H:i:s') ?>
            </div>
        </div>
    </div>
</body>
</html>
<?php
odbc_close($conn);
?>