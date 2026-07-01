<?php
if (PT!=1) exit;

// Incluir arquivos de configuração e segurança
include_once "incluir/configura.php";
include_once "injection.php";

header('Content-Type: text/html; charset=utf-8');

// Função para conectar usando ODBC
function conectarBanco() {
    global $host, $banco, $user, $pass;
    $dsn = "Driver={SQL Server};Server=$host;Database=$banco;";
    $conn = odbc_connect($dsn, $user, $pass);
    if (!$conn) {
        throw new Exception("Erro ao conectar via ODBC: " . odbc_errormsg());
    }
    return $conn;
}

// Função para verificar se o usuário existe
function verificarUsuario($conn, $userID) {
    $sql = "SELECT UserID, UserCoin FROM dbo.UserInfo WHERE UserID = ?";
    $stmt = odbc_prepare($conn, $sql);
    if (!$stmt) return false;
    $result = odbc_execute($stmt, array($userID));
    if (!$result) return false;
    if ($row = odbc_fetch_array($stmt)) {
        return array(
            'UserID' => $row['UserID'],
            'UserCoin' => $row['UserCoin']
        );
    }
    return false;
}

// Função para remover coins
function removerCoins($conn, $userID, $coins) {
    $data_atual = date('Y-m-d H:i:s');
    $sql = "UPDATE dbo.UserInfo SET UserCoin = UserCoin - ?, LastChange = ? WHERE UserID = ?";
    $stmt = odbc_prepare($conn, $sql);
    if (!$stmt) return false;
    return odbc_execute($stmt, array($coins, $data_atual, $userID));
}

// Função para obter saldo atual
function obterSaldo($conn, $userID) {
    $sql = "SELECT UserCoin FROM dbo.UserInfo WHERE UserID = ?";
    $stmt = odbc_prepare($conn, $sql);
    if (!$stmt) return 0;
    $result = odbc_execute($stmt, array($userID));
    if (!$result) return 0;
    if ($row = odbc_fetch_array($stmt)) {
        return $row['UserCoin'];
    }
    return 0;
}

// Função para sanitizar entrada
function sanitizarEntrada($data) {
    if (function_exists('limparString')) {
        return limparString($data);
    }
    return htmlspecialchars(trim($data), ENT_QUOTES, 'UTF-8');
}
?>

<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Sistema de Remoção de Coins - Priston Tale</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            background: url('background.gif') no-repeat center center fixed;
            background-size: cover;
            margin: 0;
            padding: 20px;
        }
        .form-table {
            width: 100%;
            max-width: 500px;
            margin: 0 auto;
            background-color: rgba(255, 255, 255, 0.8);
            padding: 20px;
            border-radius: 5px;
        }
        .form-table input[type="text"], .form-table input[type="number"] {
            width: 100%;
            padding: 8px;
            margin: 5px 0 15px;
            border: 1px solid #ccc;
            border-radius: 4px;
        }
        .form-table input[type="submit"] {
            background-color: #4CAF50;
            color: white;
            padding: 10px 15px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
        }
        .form-table input[type="submit"]:hover {
            background-color: #45a049;
        }
        .error {
            color: red;
        }
        .success {
            color: green;
        }
    </style>
    <script>
        function validarFormulario() {
            var userID = document.getElementById('userID').value.trim();
            var coins = document.getElementById('coins').value;
            if (userID === '') {
                alert('Por favor, digite o ID do usuário!');
                return false;
            }
            if (coins === '' || parseInt(coins) <= 0) {
                alert('Por favor, digite uma quantidade válida de coins!');
                return false;
            }
            return confirm('Confirma a remoção de ' + parseInt(coins).toLocaleString() + ' coins do usuário "' + userID + '"?');
        }
    </script>
</head>
<body>
<div class="container">
    <div class="header">
        <h1><span class="status-icon">??</span>Sistema de Remoção de Coins - Priston Tale</h1>
        <p>Remover coins diretamente no banco CoinShopDB</p>
    </div>
    <div class="form-section">
        <h3><span class="status-icon">??</span>Remover Coins do Usuário</h3>

        <form method="post" action="" onsubmit="return validarFormulario()">
            <table class="form-table">
                <tr>
                    <td>ID do Usuário:</td>
                    <td>
                        <input type="text" name="userID" id="userID" placeholder="Digite o ID do usuário" required />
                    </td>
                </tr>
                <tr>
                    <td>Coins a Remover:</td>
                    <td>
                        <input type="number" name="coins" id="coins" min="1" max="2000000000" placeholder="Ex: 1000" required />
                    </td>
                </tr>
                <tr>
                    <td colspan="2" style="text-align: center; padding-top: 20px;">
                        <input type="submit" name="enviar" class="button" value="?? Remover Coins" />
                    </td>
                </tr>
            </table>
        </form>
    </div>
    <?php
    if (isset($_POST['enviar'])) {
        $userID = sanitizarEntrada($_POST['userID']);
        $coins = intval($_POST['coins']);

        echo "<div class='form-section'>";
        echo "<h3><span class='status-icon'>??</span>Resultado da Operação</h3>";

        if (empty($userID) || $coins <= 0) {
            echo "<div class='error'>";
            echo "<span class='status-icon'>?</span>";
            echo "<strong>Erro de Validação:</strong><br>";
            echo "• ID do usuário não pode estar vazio<br>";
            echo "• Quantidade de coins deve ser maior que 0";
            echo "</div>";
        } else {
            try {
                $conn = conectarBanco();
                echo "<div class='info'>";
                echo "<span class='status-icon'>?</span>";
                echo "<strong>Conexão estabelecida com sucesso!</strong><br>";
                echo "Database: CoinShopDB | Table: dbo.UserInfo";
                echo "</div>";

                $usuario = verificarUsuario($conn, $userID);
                if (!$usuario) {
                    echo "<div class='error'>";
                    echo "<span class='status-icon'>??</span>";
                    echo "<strong>Usuário não encontrado!</strong><br>";
                    echo "O usuário '<em>" . htmlspecialchars($userID) . "</em>' não existe no banco de dados.";
                    echo "</div>";
                } else {
                    $saldo_antes = $usuario['UserCoin'];
                    if ($saldo_antes < $coins) {
                        echo "<div class='error'>";
                        echo "<span class='status-icon'>?</span>";
                        echo "<strong>Erro ao remover coins!</strong><br>";
                        echo "O usuário não tem coins suficientes para remover.";
                        echo "</div>";
                    } else {
                        if (removerCoins($conn, $userID, $coins)) {
                            $saldo_depois = obterSaldo($conn, $userID);
                            echo "<div class='success'>";
                            echo "<span class='status-icon'>??</span>";
                            echo "<strong>Coins removidos com sucesso!</strong>";
                            echo "<table class='result-table'>";
                            echo "<tr><th>Informação</th><th>Valor</th></tr>";
                            echo "<tr><td><strong>User ID</strong></td><td>" . htmlspecialchars($userID) . "</td></tr>";
                            echo "<tr><td><strong>Saldo Anterior</strong></td><td>" . number_format($saldo_antes) . " coins</td></tr>";
                            echo "<tr><td><strong>Coins Removidos</strong></td><td>- " . number_format($coins) . " coins</td></tr>";
                            echo "<tr><td><strong>Saldo Atual</strong></td><td>" . number_format($saldo_depois) . " coins</td></tr>";
                            echo "<tr><td><strong>Data/Hora</strong></td><td>" . date('d/m/Y H:i:s') . "</td></tr>";
                            echo "</table>";
                            echo "</div>";
                        } else {
                            echo "<div class='error'>";
                            echo "<span class='status-icon'>?</span>";
                            echo "<strong>Erro ao remover coins!</strong><br>";
                            echo "Verifique os logs do sistema para mais detalhes.";
                            echo "</div>";
                        }
                    }
                }
                odbc_close($conn);
            } catch (Exception $e) {
                echo "<div class='error'>";
                echo "<span class='status-icon'>??</span>";
                echo "<strong>Erro de Conexão:</strong> " . htmlspecialchars($e->getMessage());
                echo "<br><br><strong>Possíveis soluções:</strong>";
                echo "<ul>";
                echo "<li>Verificar se o SQL Server está rodando</li>";
                echo "<li>Confirmar se o banco CoinShopDB existe</li>";
                echo "<li>Verificar configurações no arquivo configura.php</li>";
                echo "<li>Verificar permissões do usuário do banco</li>";
                echo "</ul>";
                echo "</div>";
            }
        }
        echo "</div>";
    }
    ?>
    <div class="instructions">
        <h3><span class="status-icon">??</span>Instruções de Uso</h3>
        <ol>
            <li><strong>Digite o ID do usuário</strong> que terá os coins removidos.</li>
            <li><strong>Informe a quantidade</strong> de coins a remover.</li>
            <li><strong>Clique em "Remover Coins"</strong> para executar.</li>
        </ol>
        <div style="background: #fff3cd; padding: 15px; border-radius: 8px; margin-top: 15px; border-left: 4px solid #ffc107;">
            <h4>?? Informações Importantes:</h4>
            <ul>
                <li>Os coins são <strong>REMOVIDOS</strong> do saldo existente.</li>
                <li>A data/hora da operação é registrada automaticamente.</li>
                <li>Sistema usa configurações do arquivo configura.php.</li>
                <li>Banco: <code>CoinShopDB.dbo.UserInfo</code>.</li>
                <li>Proteção contra SQL Injection ativada.</li>
            </ul>
        </div>
    </div>
</div>
</body>
</html>
