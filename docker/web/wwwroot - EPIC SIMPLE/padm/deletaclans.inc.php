<?
if (PT!=1) exit;
?>
<?
include_once "injection.php";
include_once "incluir/configura.php";
?>
<style type="text/css">
<!--
.style2 {font-family: Verdana, Arial, Helvetica, sans-serif}
-->
</style>
<link href="css/style.css" rel="stylesheet" type="text/css">
<table background="imgs/fundo_textura1.gif" width="712" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td width="712"><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
                  <tr>
                    <td colspan="2" align="center" bgcolor="#000000"><b><font color="#FFFFFF">Deletar todos clans do servidor </font></b></td>
                  </tr>
<link href="style.css" rel="stylesheet" type="text/css">
<body background="background.gif">
<form id="form1" name="form1" method="post" action="">
<table width="235" height="81" border="0" align="center" cellpadding="0" cellspacing="0">
</table>
<br>
  <table width="721" height="28" border="0" align="center" cellpadding="0" cellspacing="0">
    <tr>
      <th width="721" scope="row"><span class="style2">Deletando todos clans do servidor</span></th>
    </tr>
    <tr>
      <th scope="row"><label>
        <input type="hidden" name="acao" value="nick"><input type="submit" class="button6" onclick ="return confirm('Você tem certeza que deseja, deletar todos clans do servidor!?')" value="Deletar todos clans"><br>
        <input name="acao2" type="hidden" id="acao2" value="deletclans" />
        <br />
        <?
	  if($_POST[acao2] == "deletclans")
	  {
if($_SESSION['permissao'] < 3){
echo "Você não tem permissão para acessar esta área";
exit;
}

$conn = odbc_connect($clan,$user,$pass) or die("Erro ao conectar");

$seleciona = "SELECT * FROM dbo.CT";
$seleciona1 = "SELECT * FROM dbo.UL";

$sql = odbc_exec($conn,$seleciona,$seleciona1);
odbc_do($conn,"DELETE FROM dbo.CL");
odbc_do($conn,"DELETE FROM dbo.UL");
echo"<font color='red'<b>Todos os clans forão deletados com sucesso!<br>";
}
?>
      </label></th>
    </tr>
    <tr>
                  <td colspan="2" align="center"><div align="left">
                    <p><strong>Obs: cuidado ao usar está ferramenta.</strong>                    
                    <p>Essa ferramenta, da acesso a voc&ecirc; clicando em deletar todos clans vai deletar todos clan do servidor automaticamente n&atilde;o ficara nenhum.<strong><br>
                    </strong>Ferramenta protegida com confirmação de deletação, se por acaso você clicar em deletar todos clans sem querer, aparecera uma mensagem se realmente deseja fazer está ação basta clicar cancelar, caso deseja fazer a ação clique em Ok, sera excluido todos clan do servidor.<br> 
                    </table>
      </div></td>
    </tr>
  </table>
</form>
<form id="form2" name="form2" method="post" action="">
</form>
<? ?>    