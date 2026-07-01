<?php
// ConfiguraÃƒÂ§ÃƒÂµes
$server = 'PTL\SQLEXPRESS';
$database = 'ClanDB';
$username = 'sa';
$password = '#$pdl32xq';
$websiteUrl = 'https://summittale.com.br/';
$imagesPath = '/ClanContent'; // Ajustado para o caminho dos ÃƒÂ­cones de clÃƒÂ£

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

// Consulta SQL para ranking de clÃƒÂ£s por SOD
$sql = "SELECT TOP 100 
           [MIconCnt] AS Icone,
           [ClanName] AS NomeClan, 
           [ClanZang] AS Lider, 
           [MemCnt] AS Membros,
           [Cpoint] AS PontosSOD
        FROM [CL] 
        ORDER BY [Cpoint] DESC";

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
    <title>Ranking de Clans - Summit Tale</title>
    <style>
        :root {
            --primary-color: #8B0000;  /* Vermelho escuro para clÃƒÂ£s */
            --secondary-color: #6c757d;
            --accent-color: #FFD700;   /* Dourado */
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
            background: linear-gradient(135deg, var(--primary-color), #6B0000);
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
        
        .clan-info {
            display: flex;
            align-items: center;
            gap: 12px;
        }
        
        .clan-icon {
            width: 32px;
            height: 32px;
            object-fit: contain;
            border-radius: 50%;
            border: 2px solid var(--accent-color);
        }
        
        .clan-name {
            font-weight: 500;
        }
        
        .clan-leader {
            font-size: 12px;
            color: var(--secondary-color);
        }
        
        .clan-members {
            text-align: center;
            font-weight: 600;
        }
        
        .clan-points {
            font-weight: 700;
            color: var(--primary-color);
            text-align: right;
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
            
            .clan-info {
                gap: 8px;
            }
            
            .clan-icon {
                width: 24px;
                height: 24px;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="ranking-card">
            <div class="ranking-header">
                <h1>Ranking de Clans</h1>
                <p>Top 100 Clans por SOD</p>
            </div>
            
            <table class="ranking-table">
                <thead>
                    <tr>
                        <th>#</th>
                        <th>Clan</th>
                        <th>Lider</th>
                        <th>Membros</th>
                        <th>SOD</th>
                    </tr>
                </thead>
                <tbody>
                    <?php
                    $position = 1;
                    while ($row = odbc_fetch_array($result)) {
                        echo "<tr>
                                <td class='rank'>{$position}</td>
                                <td>
                                    <div class='clan-info'>
                                        <img src='{$websiteUrl}{$imagesPath}/{$row['Icone']}.bmp' 
                                             alt='Emblema do ClÃƒÂ£' class='clan-icon'>
                                        <span class='clan-name'>".htmlspecialchars($row['NomeClan'])."</span>
                                    </div>
                                </td>
                                <td class='clan-leader'>".htmlspecialchars($row['Lider'])."</td>
                                <td class='clan-members'>{$row['Membros']}</td>
                                <td class='clan-points'>".number_format($row['PontosSOD'])."</td>
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