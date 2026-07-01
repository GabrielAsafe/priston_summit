<?
include "incluir/configura.php";
function navegador()
{
$navegador = $_SERVER['HTTP_USER_AGENT'];
$nome = strlen("Mozilla");
$i = substr($navegador,0,$nome);
$ok = "Mozilla";
	if($i == $ok){
		return false;
	}else{
		echo "O uso do Mozilla Firefox e OBRIGATORIO para acessar esta pagina.";
		exit;
	}
}
navegador();
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<title><?=$version?></title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<link href="css/style.css" rel="stylesheet" type="text/css">
<style type="text/css">
<!--
body {
	margin-top: 30px;
}
-->
</style></head>
<body background="imgs/fundo_cinza_conteudos.jpg" leftmargin="0" marginwidth="0" allowtransparency="true">
      <table width="800" border="0" align="center" cellpadding="0" cellspacing="0">
                <tr>
          <td><img src="imgs/banner.gif"></td>
        </tr>
        </table>
        <div align="center">
          <table width="808" bgcolor="#c0c0c0" border="1" bordercolor="#333333" cellpadding="0" cellspacing="0">
            <tr>
              <td align="center"><font color="#008000"><b><br><br>DESLOGADO COM SUCESSO!!!<br> REDIRECIONANDO...<br><br></b></font></td>
            </tr>
            </table>
        </div>
      <table width="800" border="0" align="center" cellpadding="0" cellspacing="0">
        <tr>
          <td><img src="imgs/rodape.gif" width="808" height="20"></td>
        </tr>
      </table>
      <meta HTTP-EQUIV="Refresh" CONTENT="3;URL=index.php">
      <br>
</body>
</html>