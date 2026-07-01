<?php
if (PT != 1) exit;

// Incluir arquivos de configuração e segurança
include_once "configura.php";
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

// Função para obter saldo atual
function obterSaldo($conn, $userID) {
    $sql = "SELECT UserCoin FROM dbo.UserInfo WHERE UserID = ?";
    $stmt = odbc_prepare($conn, $sql);
    if (!$stmt) return false;
    $result = odbc_execute($stmt, array($userID));
    if (!$result) return false;
    if ($row = odbc_fetch_array($stmt)) {
        return $row['UserCoin'];
    }
    return false;
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
    <title>Verificar Coins do Usuário - Priston Tale</title>
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
        .form-table input[type="text"] {
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
</head>
<body>
<div class="container">
    <div class="header">
        <h1><span class="status-icon">🎮</span>Verificar Coins do Usuário - Priston Tale</h1>
        <p>Verifique a quantidade de coins de um usuário no banco CoinShopDB</p>
    </div>
    <div class="form-section">
        <h3><span class="status-icon">💰</span>Verificar Coins</h3>

        <form method="post" action="">
            <table class="form-table">
                <tr>
                    <td>ID do Usuário:</td>
                    <td>
                        <input type="text" name="userID" id="userID" placeholder="Digite o ID do usuário" required />
                    </td>
                </tr>
                <tr>
                    <td colspan="2" style="text-align: center; padding-top: 20px;">
                        <input type="submit" name="enviar" class="button" value="🔍 Verificar Coins" />
                    </td>
                </tr>
            </table>
        </form>
    </div>
    <?php
    if (isset($_POST['enviar'])) {
        $userID = sanitizarEntrada($_POST['userID']);

        echo "<div class='form-section'>";
        echo "<h3><span class='status-icon'>📊</span>Resultado da Consulta</h3>";

        if (empty($userID)) {
            echo "<div class='error'>";
            echo "<span class='status-icon'>❌</span>";
            echo "<strong>Erro de Validação:</strong><br>";
            echo "• ID do usuário não pode estar vazio";
            echo "</div>";
        } else {
            try {
                $conn = conectarBanco();
                $saldo = obterSaldo($conn, $userID);

                if ($saldo === false) {
                    echo "<div class='error'>";
                    echo "<span class='status-icon'>👤</span>";
                    echo "<strong>Usuário não encontrado!</strong><br>";
                    echo "O usuário '<em>" . htmlspecialchars($userID) . "</em>' não existe no banco de dados.";
                    echo "</div>";
                } else {
                    echo "<div class='success'>";
                    echo "<span class='status-icon'>💰</span>";
                    echo "<strong>Saldo de Coins:</strong>";
                    echo "<table class='result-table'>";
                    echo "<tr><th>User ID</th><th>Saldo de Coins</th></tr>";
                    echo "<tr><td>" . htmlspecialchars($userID) . "</td><td>" . number_format($saldo) . " coins</td></tr>";
                    echo "</table>";
                    echo "</div>";
                }

                odbc_close($conn);
            } catch (Exception $e) {
                echo "<div class='error'>";
                echo "<span class='status-icon'>🔌</span>";
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
        <h3><span class="status-icon">📋</span>Instruções de Uso</h3>
        <ol>
            <li><strong>Digite o ID do usuário</strong> para verificar o saldo de coins.</li>
            <li><strong>Clique em "Verificar Coins"</strong> para executar a consulta.</li>
        </ol>
    </div>
</div>
</body>
</html>
