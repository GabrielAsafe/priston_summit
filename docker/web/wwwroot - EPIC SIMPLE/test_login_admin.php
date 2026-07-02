<?php
define("PT", "1");
chdir(__DIR__ . "/padm");
include "incluir/configura.php";

$u = "admin";
$p = "gm";

echo "<pre>";
echo "connection_string=$connection_string\n";
echo "user=$user\n";

$c = odbc_connect($connection_string, $user, $pass);
var_dump($c);

$sql = "SELECT * FROM [ADM].[dbo].[loginGM] WHERE [idGM]='$u' AND [passGM]='$p'";
echo "SQL=$sql\n";

$q = odbc_exec($c, $sql);
if (!$q) {
    echo "ODBC ERROR: " . odbc_errormsg($c) . "\n";
    exit;
}

$r = odbc_fetch_array($q);
var_dump($r);
echo "</pre>";
