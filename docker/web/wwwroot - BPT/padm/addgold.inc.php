<?if (PT!=1) exit;?>

<?
include_once "incluir/configura.php";
include_once "injection.php";
?>
<style type="text/css">
<!--
.style2 {font-family: Verdana, Arial, Helvetica, sans-serif}
-->
</style>
<link href="css/style.css" rel="stylesheet" type="text/css">
<table background="imgs/fundo_textura1.gif" width="716" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td width="716"><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
<table width="550" border="0">
              
                     <td width="888" align="center" bgcolor="#000000"><b><font color="#FFFFFF">Adicionando gold no banco de Gold do painel Player</font></b></td>
                  </tr></table>
<form method="post" action="index.php?sessadm=deposita">
  <p>ID do Player:    &nbsp;
    <input type="text" name="ID2" id="ID2"><br /><br />
  GOLD VIRTUAL:
  <input type="text" name="quantia2" id="quantia2">
  </p>
  <p><strong>Exemplos</strong>:1000000= 1kk, 10000000=10kk, 50000000=50kk,100000000=100kk<br />
    Ps: coloque no campo de gold virtual quantidade exemplo 
  1000000 que equivale 1kk n&atilde;o coloque assim 1kk coloque, 1000000 *Ex.<br><br />
    <input type="submit" class="button6" id="depositar" value="Depositar no Banco" />
  </p>
<p>&nbsp;</p>
  <p><strong>Instru&ccedil;&otilde;es de como Utilizar o Sistema de Adicionar gold no banco de gold de player.<br />
  </strong>1 Passo - Digite o id do player que voc&ecirc; deseja mandar o gold para o banco virtual.<br />
2 Passo - Em GOLD VIRTUAL coloque a quantidade de gold que deseja mandar para o banco virtual do player.<br />
<strong>Exemplos</strong>:1000000= 1kk, 10000000=10kk, 50000000=50kk,100000000=100kk<br />
3 Passo - Clique em depositar no banco, vai falar que o gold foi depositado com sucesso no banco virtual do player so comunicar o player pronto.</p>
</form>
</table>

<?  ?>   
