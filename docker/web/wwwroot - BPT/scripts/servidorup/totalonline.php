<?php
ob_start( 'ob_gzhandler' );
require "config.php";
function AcertarLinhasODBC($string,$query){
	$resultado = odbc_exec($string,$query);
	$contador=0;
	while($temp = odbc_fetch_into($resultado, &$counter)){
		$contador++;
	}
	return $contador;
}

$string = odbc_connect($Conexao_ClanDB, $Usuario_SQL, $Senha_SQL);
$query1= "SELECT * FROM [ClanDB].[dbo].[CT]";
$excuta_query1 = odbc_exec($string, $query1);
$contador_total= AcertarLinhasODBC($string, $query1);
?>

<link href="<? echo $Website; ?>/css/scripts.css" rel="stylesheet" type="text/css">
<body style="background-color:transparent">
<tr>
<td>
<div id="serverstatus">

<font color="#FFFFFF">Usuários online no servidor:</font> <font color="#006400"><b><?php echo $contador_total?></font></b>

</tr>
</td>
</div>
</body>

<?php
if($contador_total=="0"){
}elseif($contador_total!="0"){}
?>