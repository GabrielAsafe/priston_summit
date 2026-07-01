<?php
if (PT!=1) exit;

// Incluir arquivos de configuração e segurança
include_once "incluir/configura.php";
include_once "injection.php";

header('Content-Type: text/html; charset=utf-8');

// Função para conectar usando ODBC
function conectarBanco() {
    global $host, $banco, $user, $pass;

    // Montar a string de conexão ODBC
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

// Função para adicionar coins
function adicionarCoins($conn, $userID, $coins) {
    $data_atual = date('Y-m-d H:i:s');
    $sql = "UPDATE dbo.UserInfo SET UserCoin = UserCoin + ?, LastChange = ? WHERE UserID = ?";
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

// Função para criar usuário
function criarUsuario($conn, $userID, $coins) {
    $data_atual = date('Y-m-d H:i:s');
    $sql = "INSERT INTO dbo.UserInfo (UserID, UserCoin, LastChange) VALUES (?, ?, ?)";
    $stmt = odbc_prepare($conn, $sql);
    if (!$stmt) return false;
    return odbc_execute($stmt, array($userID, $coins, $data_atual));
}

// Função para buscar usuários (autocomplete)
function buscarUsuarios($conn, $termo = '') {
    if (strlen($termo) >= 2) {
        $termo_like = "%$termo%";
        $sql = "SELECT TOP 10 UserID, UserCoin FROM dbo.UserInfo WHERE UserID LIKE ? ORDER BY UserID";
        $stmt = odbc_prepare($conn, $sql);
        if (!$stmt) return array();
        odbc_execute($stmt, array($termo_like));
        
        $usuarios = array();
        while ($row = odbc_fetch_array($stmt)) {
            $usuarios[] = array(
                'id' => $row['UserID'],
                'coins' => $row['UserCoin']
            );
        }
        return $usuarios;
    }
    return array();
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
    <title>Sistema de Coins - Priston Tale</title>
    <style>
        /* (Seu CSS continua igual, mantenha tudo igual aqui) */
        body { 
            font-family: Arial, sans-serif; 
            background: url('background.gif') no-repeat center center fixed;
            background-size: cover;
            margin: 0;
            padding: 20px;
        }
        /* ... resto do CSS igual ... */
    </style>
    <script>
        // Função para buscar usuários (simulação autocomplete)
        function buscarUsuario() {
            var input = document.getElementById('userID');
            var termo = input.value;
            if (termo.length >= 2) {
                console.log('Buscando usuários com termo: ' + termo);
                // Implementar AJAX real se quiser
            }
        }
        // Validação do formulário
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
            return confirm('Confirma a adição de ' + parseInt(coins).toLocaleString() + ' coins para o usuário "' + userID + '"?');
        }
    </script>
</head>
<body>

<div class="container">
    <div class="header">
        <h1><span class="status-icon">🎮</span>Sistema de Coins - Priston Tale</h1>
        <p>Adicionar coins diretamente no banco CoinShopDB</p>
    </div>

    <div class="form-section">
        <h3><span class="status-icon">💰</span>Adicionar Coins ao Usuário</h3>
        
        <form method="post" action="" onsubmit="return validarFormulario()">
            <table class="form-table">
                <tr>
                    <td>ID do Usuário:</td>
                    <td>
                        <input type="text" name="userID" id="userID" placeholder="Digite o ID do usuário" 
                               onkeyup="buscarUsuario()" required />
                        <div id="searchResults" class="search-results"></div>
                    </td>
                </tr>
                <tr>
                    <td>Coins a Adicionar:</td>
                    <td>
                        <input type="number" name="coins" id="coins" min="1" max="2000000000" 
                               placeholder="Ex: 1000" required />
                    </td>
                </tr>
                <tr>
                    <td>Opções:</td>
                    <td>
                        <label style="margin-right: 20px;">
                            <input type="checkbox" name="criar_usuario" value="1" />
                            Criar usuário se não existir
                        </label>
                        <label>
                            <input type="checkbox" name="confirmar" value="1" />
                            Confirmar operação
                        </label>
                    </td>
                </tr>
                <tr>
                    <td colspan="2" style="text-align: center; padding-top: 20px;">
                        <input type="submit" name="enviar" class="button" value="💎 Adicionar Coins" />
                    </td>
                </tr>
            </table>
        </form>
    </div>

<?php
if (isset($_POST['enviar'])) {
    $userID = sanitizarEntrada($_POST['userID']);
    $coins = intval($_POST['coins']);
    $criar_usuario = isset($_POST['criar_usuario']);
    $confirmar = isset($_POST['confirmar']);

    echo "<div class='form-section'>";
    echo "<h3><span class='status-icon'>📊</span>Resultado da Operação</h3>";

    if (empty($userID) || $coins <= 0) {
        echo "<div class='error'>";
        echo "<span class='status-icon'>❌</span>";
        echo "<strong>Erro de Validação:</strong><br>";
        echo "• ID do usuário não pode estar vazio<br>";
        echo "• Quantidade de coins deve ser maior que 0";
        echo "</div>";
    } else if (!$confirmar) {
        echo "<div class='error'>";
        echo "<span class='status-icon'>⚠️</span>";
        echo "<strong>Confirmação Necessária:</strong><br>";
        echo "Por favor, marque a opção 'Confirmar operação' para prosseguir.";
        echo "</div>";
    } else {
        try {
            $conn = conectarBanco();
            echo "<div class='info'>";
            echo "<span class='status-icon'>✅</span>";
            echo "<strong>Conexão estabelecida com sucesso!</strong><br>";
            echo "Database: CoinShopDB | Table: dbo.UserInfo";
            echo "</div>";

            $usuario = verificarUsuario($conn, $userID);

            if (!$usuario) {
                if ($criar_usuario) {
                    if (criarUsuario($conn, $userID, $coins)) {
                        echo "<div class='success'>";
                        echo "<span class='status-icon'>🎉</span>";
                        echo "<strong>Novo usuário criado com sucesso!</strong>";
                        echo "<table class='result-table'>";
                        echo "<tr><th>Campo</th><th>Valor</th></tr>";
                        echo "<tr><td><strong>User ID</strong></td><td>" . htmlspecialchars($userID) . "</td></tr>";
                        echo "<tr><td><strong>Coins Iniciais</strong></td><td>" . number_format($coins) . "</td></tr>";
                        echo "<tr><td><strong>Data/Hora</strong></td><td>" . date('d/m/Y H:i:s') . "</td></tr>";
                        echo "</table>";
                        echo "</div>";
                    } else {
                        echo "<div class='error'>";
                        echo "<span class='status-icon'>❌</span>";
                        echo "<strong>Erro ao criar usuário!</strong><br>";
                        echo "Verifique os logs do sistema para mais detalhes.";
                        echo "</div>";
                    }
                } else {
                    echo "<div class='error'>";
                    echo "<span class='status-icon'>👤</span>";
                    echo "<strong>Usuário não encontrado!</strong><br>";
                    echo "O usuário '<em>" . htmlspecialchars($userID) . "</em>' não existe no banco de dados.<br>";
                    echo "<em>💡 Dica: Marque a opção 'Criar usuário se não existir' para criar automaticamente.</em>";
                    echo "</div>";
                }
            } else {
                $saldo_antes = $usuario['UserCoin'];
                if (adicionarCoins($conn, $userID, $coins)) {
                    $saldo_depois = obterSaldo($conn, $userID);
                    echo "<div class='success'>";
                    echo "<span class='status-icon'>💰</span>";
                    echo "<strong>Coins adicionados com sucesso!</strong>";
                    echo "<table class='result-table'>";
                    echo "<tr><th>Informação</th><th>Valor</th></tr>";
                    echo "<tr><td><strong>User ID</strong></td><td>" . htmlspecialchars($userID) . "</td></tr>";
                    echo "<tr><td><strong>Saldo Anterior</strong></td><td>" . number_format($saldo_antes) . " coins</td></tr>";
                    echo "<tr><td><strong>Coins Adicionados</strong></td><td>+" . number_format($coins) . " coins</td></tr>";
                    echo "<tr><td><strong>Saldo Atual</strong></td><td>" . number_format($saldo_depois) . " coins</td></tr>";
                    echo "<tr><td><strong>Data/Hora</strong></td><td>" . date('d/m/Y H:i:s') . "</td></tr>";
                    echo "</table>";
                    echo "</div>";
                } else {
                    echo "<div class='error'>";
                    echo "<span class='status-icon'>❌</span>";
                    echo "<strong>Erro ao adicionar coins!</strong><br>";
                    echo "Verifique os logs do sistema para mais detalhes.";
                    echo "</div>";
                }
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
        <li><strong>Digite o ID do usuário</strong> que receberá os coins</li>
        <li><strong>Informe a quantidade</strong> de coins a adicionar (será somada ao saldo atual)</li>
        <li><strong>Opcionalmente</strong> marque para criar usuário se ele não existir</li>
        <li><strong>Marque "Confirmar operação"</strong> para validar a ação</li>
        <li><strong>Clique em "Adicionar Coins"</strong> para executar</li>
    </ol>

    <div style="background: #fff3cd; padding: 15px; border-radius: 8px; margin-top: 15px; border-left: 4px solid #ffc107;">
        <h4>⚠️ Informações Importantes:</h4>
        <ul>
            <li>Os coins são <strong>SOMADOS</strong> ao saldo existente</li>
            <li>A data/hora da operação é registrada automaticamente</li>
            <li>Sistema usa configurações do arquivo configura.php</li>
            <li>Banco: <code>CoinShopDB.dbo.UserInfo</code></li>
            <li>Proteção contra SQL Injection ativada</li>
        </ul>
    </div>
</div>

</div>

</body>
</html>
