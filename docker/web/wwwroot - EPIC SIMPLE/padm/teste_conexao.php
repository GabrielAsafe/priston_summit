<?php
include_once "incluir/configura.php";

$conn = odbc_connect($connection_string, $user, $pass);

if ($conn) {
    echo "Conexão OK via SummitSQL";
    odbc_close($conn);
} else {
    echo "Erro ODBC: " . odbc_errormsg();
}
