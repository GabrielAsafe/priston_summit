
<?
if (PT!=1)  exit;
?>
<link href="css/style.css" rel="stylesheet" type="text/css">
<table background="imgs/fundo_textura1.gif" width="712" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td width="712"><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
                  <tr>
<?
$conexao = odbc_connect($connection_string, $user, $pass) or die ("Erro ao Conectar");


	if($_POST['action']=='act'){
		if($_POST['Pbanir']=='1'){
			$act='Banido';
			$motivo = $_POST['Motivo'];
		}else{
			$act='Desbanido';
			$motivo='';
		}
		
		$datahoje = date("m-d-Y");
		$query = odbc_do($conexao, "UPDATE dbo.AllGameUser SET BlockChk='$_POST[Pbanir]', EditDay='$datahoje', RNo='$motivo' WHERE userid='$_POST[Pname]'");
		$query2 = odbc_do($conexao, "UPDATE dbo.".strtoupper($_POST[Pname]{0})."GameUser SET BlockChk='$_POST[Pbanir]' WHERE userid='$_POST[Pname]'");
		if($query and $query2){
			echo "<p class='textoPPP' align='center'>Conta $_POST[Pname], foi $act</p>";
				 "<script type=\"text/javascript\">".
				 "alert(\"Logout Efetuado!\");".
				 "window.location=\"index.php?".$_SERVER["QUERY_STRING"]."\";".
				 "</script>";
		}else{
			echo "<p class='textoPPP' align='center'>$_POST[Pname], não foi $act</p>".
				 "<script type=\"text/javascript\">".
				 "alert(\"Logout Efetuado!\");".
				 "window.location=\"index.php?".$_SERVER["QUERY_STRING"]."\";".
				 "</script>";
		}
	}else{
?>

<?php
	}
?>
<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#330099">
  <?  ?>
</table>
<p align="center"><strong>Lista de todos jogadores banidos do servidor</strong></p>
<table width="90%" align="center" cellpadding="3" cellspacing="3" id="rank" >
<tr>
    <td align="center" bgcolor="#000000"><font color="#FFFFFF">ID do Player</font></td>
    <td align="center" bgcolor="#000000"><font color="#FFFFFF">Puni&ccedil;&atilde;o</font></td>
    
  </tr>
<?php
		$selecionaBannidos = "SELECT TOP 1000000000 userid, RNo, EditDay FROM AllGameUser WHERE BlockChk='1'";
		$excuta_selecionaBannidos = odbc_exec($conexao, $selecionaBannidos);
        $i=1;
        while($pega_busca=odbc_fetch_array($excuta_selecionaBannidos)){
        foreach ($pega_busca as $key => $value) { 
        $$key=(IS_ARRAY($value))?$value:utf8_decode($value);
        }

?>
  <tr>
    <td align="center"><span class="textoPPP">
      <?=$userid?>
    </span>&nbsp;</td>
    <td align="center">BANIDO&nbsp;</td>
    
  </tr>
<?php

	}
?>
  </table>
</table>
<?php
?>
