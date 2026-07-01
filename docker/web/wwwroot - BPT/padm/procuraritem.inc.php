<?
if (PT!=1) exit;
 
?>
<?
include_once "injection.php";
?>
<link href="css/style.css" rel="stylesheet" type="text/css" />
<br>
	<table background="imgs/fundo_textura1.gif" width="723" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td width="723"><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
                  <tr>
             
                    <td width="704" align="center" bgcolor="#000000"><b><font color="#FFFFFF"> Procurar ITEMS em um Personagem</font></b></td>
                  </tr></table>
<form id="form1" name="form1" method="post" action="index.php?sessadm=itemresultado">
                    <p align="center">Nick do Player:
                      <input type="text" name="id" id="id" />
  </p>
                    <p align="center">Nome do Item:
                      <input type="text" name="iten" id="iten" />
                    </p>
                    <p align="center">
                      <input name="button" type="submit" class="button6" id="button" value="Procurar" />
                    </p>
</form>
<table width="80%" border="0" align="center" cellpadding="4" cellspacing="2">
  <tr>
    <td colspan="2" align="center"><div align="center">
      <p><strong>Instru&ccedil;&otilde;es de como Utilizar o Sistema de Procurar item em um char<br />
        </strong>1 Passo - Procure saber o nick do player que voc&ecirc; quer verifica se possui o item x.<br />
        2 Passo - Digite o nick do player no campo de cima e no de baixo o nome do item certinho com letras maiusculas e minusculas apos isso clique em Procurar.<br />
        quando voc&ecirc; clicar em procurar vai aparecer se existe o item x no player ou n&atilde;o.<br />
        </table>
        <br /></td>
  </tr>
</table>
<p>
  <?  ?>
</p>
<p>&nbsp; </p>                