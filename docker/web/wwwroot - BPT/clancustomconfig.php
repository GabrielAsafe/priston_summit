<?php
/**
 * Configurações do Sistema de Customização de Clan - Summit Tale
 * PHP 5.3 Compatible
 */

// Definir timezone (igual ao donate)
date_default_timezone_set('America/Sao_Paulo');

// Configurações do Banco de Dados SQL Server (EXATAMENTE igual ao seu sistema)
define('DB_HOST', '127.0.0.1');
define('DB_PORT', '49684');
define('DB_USER', 'sa');
define('DB_PASSWORD', '#$pdl32xq');
define('DB_NAME_ACCOUNT', 'AccountDB');
define('DB_NAME_CLAN', 'ClanDB');

// Variáveis globais como no seu sistema
$connection_string = "Driver={SQL Server};Server=" . DB_HOST . "," . DB_PORT . ";";
$db_username = DB_USER;
$db_password = DB_PASSWORD;

// Configurações do Clan
define('CLAN_IMAGES_URL', 'https://summittale.com.br/ClanContent/');
define('TOTAL_CLAN_IMAGES', 40);
define('IMAGES_PER_PAGE', 25);

/**
 * Conectar ao SQL Server - EXATAMENTE como no seu sistema de cadastro
 */
function conectarBanco($database = null) {
    global $connection_string, $db_username, $db_password;
    
    if (!function_exists('odbc_connect')) {
        return false;
    }
    
    try {
        // Conectar como no seu sistema de cadastro
        $connection = odbc_connect($connection_string, $db_username, $db_password);
        
        if ($connection && $database) {
            // Mudar para o database específico
            $result = @odbc_exec($connection, "USE [$database]");
            if (!$result) {
                odbc_close($connection);
                return false;
            }
        }
        
        return $connection;
    } catch (Exception $e) {
        return false;
    }
}

/**
 * Verificar login do jogo - EXATAMENTE como no seu sistema
 */
function verificarLoginJogo($username, $password) {
    global $connection_string, $db_username, $db_password;
    
    try {
        // Conectar como no seu sistema
        $connection = odbc_connect($connection_string, $db_username, $db_password);
        if (!$connection) return false;
        
        // Usar AccountDB
        $useResult = @odbc_exec($connection, "USE [AccountDB]");
        if (!$useResult) {
            odbc_close($connection);
            return false;
        }
        
        // Escapar dados
        $username = str_replace("'", "''", $username);
        $password = str_replace("'", "''", $password);
        
        // Query com path completo como no debug
        $sql = "SELECT UserID, CUserName1, CUserName2 FROM [AccountDB].[dbo].[ALLPersonalMember] WHERE UserID = '$username' AND Passwd = '$password'";
        $result = odbc_exec($connection, $sql);
        
        if ($result && odbc_fetch_row($result)) {
            $data = array(
                'userId' => odbc_result($result, 'UserID'),
                'fullName' => trim(odbc_result($result, 'CUserName1') . ' ' . odbc_result($result, 'CUserName2'))
            );
            odbc_free_result($result);
            odbc_close($connection);
            return $data;
        }
        
        if ($result) odbc_free_result($result);
        odbc_close($connection);
        return false;
    } catch (Exception $e) {
        return false;
    }
}

/**
 * Verificar se é líder do clan - EXATAMENTE como no seu sistema
 */
function verificarLiderClan($userId) {
    global $connection_string, $db_username, $db_password;
    
    try {
        // Conectar como no seu sistema
        $connection = odbc_connect($connection_string, $db_username, $db_password);
        if (!$connection) return false;
        
        // Usar ClanDB
        $useResult = @odbc_exec($connection, "USE [ClanDB]");
        if (!$useResult) {
            odbc_close($connection);
            return false;
        }
        
        // Escapar dados
        $userId = str_replace("'", "''", $userId);
        
        // Query com path completo como no debug
        $sql = "SELECT ClanName, MIconCnt, Note FROM [ClanDB].[dbo].[CL] WHERE UserID = '$userId'";
        $result = odbc_exec($connection, $sql);
        
        if ($result && odbc_fetch_row($result)) {
            $data = array(
                'clanName' => odbc_result($result, 'ClanName'),
                'mIconCnt' => odbc_result($result, 'MIconCnt'),
                'note' => odbc_result($result, 'Note'),
                'userId' => $userId
            );
            odbc_free_result($result);
            odbc_close($connection);
            return $data;
        }
        
        if ($result) odbc_free_result($result);
        odbc_close($connection);
        return false;
    } catch (Exception $e) {
        return false;
    }
}

/**
 * Atualizar imagem do clan - como no seu sistema
 */
function atualizarImagemClan($clanName, $novaImagem) {
    global $connection_string, $db_username, $db_password;
    
    try {
        $connection = odbc_connect($connection_string, $db_username, $db_password);
        if (!$connection) return false;
        
        // Usar ClanDB
        odbc_exec($connection, "USE [ClanDB]");
        
        $clanName = str_replace("'", "''", $clanName);
        $novaImagem = (int)$novaImagem;
        
        $sql = "UPDATE [ClanDB].[dbo].[CL] SET MIconCnt = $novaImagem WHERE ClanName = '$clanName'";
        $result = odbc_exec($connection, $sql);
        
        odbc_close($connection);
        return $result !== false;
    } catch (Exception $e) {
        return false;
    }
}

/**
 * Atualizar frase do clan - como no seu sistema
 */
function atualizarFraseClan($clanName, $novaFrase) {
    global $connection_string, $db_username, $db_password;
    
    try {
        $connection = odbc_connect($connection_string, $db_username, $db_password);
        if (!$connection) return false;
        
        // Usar ClanDB
        odbc_exec($connection, "USE [ClanDB]");
        
        $clanName = str_replace("'", "''", $clanName);
        $novaFrase = str_replace("'", "''", $novaFrase);
        
        $sql = "UPDATE [ClanDB].[dbo].[CL] SET Note = '$novaFrase' WHERE ClanName = '$clanName'";
        $result = odbc_exec($connection, $sql);
        
        odbc_close($connection);
        return $result !== false;
    } catch (Exception $e) {
        return false;
    }
}

/**
 * Obter lista de imagens do clan
 */
function obterImagensClan() {
    $baseNumber = 1000000;
    $images = array();
    
    for ($i = 0; $i < TOTAL_CLAN_IMAGES; $i++) {
        $imageNumber = $baseNumber + $i;
        $images[] = array(
            'label' => 'Design ' . ($i + 1),
            'value' => $imageNumber,
            'imageUrl' => CLAN_IMAGES_URL . $imageNumber . '.bmp'
        );
    }
    
    return $images;
}

/**
 * Sanitizar entrada
 */
function sanitizeInput($input) {
    return htmlspecialchars(trim($input), ENT_QUOTES, 'UTF-8');
}

// Inicializar sessão (compatível com PHP 5.3)
if (session_id() == '') {
    session_start();
}
?>
