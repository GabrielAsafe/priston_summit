<?php
$dsn = 'Driver={SQL Server};Server=PTL\SQLEXPRESS;Database=CoinShopDB;';
$user = 'sa';
$pass = '#$pdl32xq';

$conn = odbc_connect($dsn, $user, $pass);
if (!$conn) {
    die("Erro ao conectar: " . odbc_errormsg());
} else {
    echo "Conectado via ODBC com sucesso!";
}
?>
