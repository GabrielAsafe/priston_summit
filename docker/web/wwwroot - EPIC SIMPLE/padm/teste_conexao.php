<?php
$conn = odbc_connect('SummitSQL', 'sa', 'SummitTale@12345');

if (!$conn) {
    die("Erro ao conectar: " . odbc_errormsg());
}

echo "Conectado via ODBC com sucesso!";
?>
