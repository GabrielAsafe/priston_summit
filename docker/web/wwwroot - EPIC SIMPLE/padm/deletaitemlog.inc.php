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
                    <td colspan="2" align="center" bgcolor="#000000"><b><font color="#FFFFFF">Deletando todos LOGs da Database ItemLogDB do Servidor </font></b></td>
                  </tr>
<link href="style.css" rel="stylesheet" type="text/css">
<body background="background.gif">
<form id="form1" name="form1" method="post" action="">
<table background="imgs/fundo_textura1.gif" width="712" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td width="712"><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
                  <tr>
      <th width="721" scope="row"><span class="style2">Deletando todos Logs da database ItemLogDB</span></th>
    </tr>
    <tr>
      <th scope="row"><label>
        <input type="hidden" name="acao" value="nick"><input type="submit" class="button6" onclick ="return confirm('Você tem certeza que deseja, deletar todos logs da database ItemLogDB?')" value="Deletar todos Log's"><br>
        <input name="acao2" type="hidden" id="acao2" value="deletarlogs" />
        <br />
        <?
		
	  if($_POST[acao2] == "deletarlogs")
	  {
	  if($_SESSION['permissao'] < 3){
echo "Você não tem permissão para acessar esta área";
exit;
}

$conn = odbc_connect($itemlog,$user,$pass) or die("Erro ao conectar");

$seleciona = "SELECT * FROM dbo.AItemCode";
$seleciona1 = "SELECT * FROM dbo.IL";
$seleciona2 = "SELECT * FROM dbo.ITEMCODE";

$sql = odbc_exec($conn,$seleciona,$seleciona1,$seleciona2);
odbc_do($conn,"DELETE FROM dbo.AItemCode");
odbc_do($conn,"DELETE FROM dbo.IL");
odbc_do($conn,"DELETE FROM dbo.ITEMCODE");
echo"<font color='red'<b>Todos os logs da database ItemLogDB forão deletados com sucesso!<br>";
}
?>
      </label></th>
    </tr>
    <tr>
                  <td colspan="2" align="center"><div align="left">
                    <p><strong>Obs: cuidado ao usar está ferramenta.</strong>                    
                    <p>Essa ferramenta, da acesso a voc&ecirc; clicando em deletar logs vai deletar todos logs da database itemlogdb do servidor automaticamente n&atilde;o ficara nenhum.<strong><br>
                    </strong>Ferramenta protegida com confirmação de deletação, se por acaso você clicar em deletar logs sem querer, aparecera uma mensagem se realmente deseja fazer está ação basta clicar cancelar, caso deseja fazer a ação clique em Ok, sera excluido todos os logs da database ItemLogDB.<br> <br>
</table>
</table>
      </div></td>
    </tr>
  </table>
</form>
<form id="form2" name="form2" method="post" action="">
</form>
<? ?>    