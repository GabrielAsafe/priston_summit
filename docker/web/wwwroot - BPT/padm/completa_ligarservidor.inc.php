<? if (PT!=1) exit;

$ipsv=$_SERVER['REMOTE_ADDR'];
$statussv = @fsockopen($ipsv, $portasv, $ERROR_NO, $ERROR_STR, (float)1.5);

	if($statussv){
	echo "<br><b>Servidor já esta em execução!!</b>";
	echo "<meta HTTP-EQUIV='Refresh' CONTENT='4;URL=?sessadm=home'>";
		} else {
	include "verificarexec.php";
	if (!$oExec++){
	echo "<br><b>Servidor Ligado com Sucesso!</b>";
	echo "<meta HTTP-EQUIV='Refresh' CONTENT='4;URL=?sessadm=home'>";
 		} else {
	echo "<br><b>Erro ao ligar o servidor!</b>";
	echo "<meta HTTP-EQUIV='Refresh' CONTENT='4;URL=?sessadm=home'>";
} }
?>
<?