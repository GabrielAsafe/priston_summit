<?if (PT!=1) exit;?>
<?php
include_once "incluir/configura.php";
if($_POST[acao]!="search")
{
?>
<style type="text/css">
<!--
.branco {
	color: #FFF;
}
.vermelho {
	color: #F00;
}
.branco {
	color: #FFF;
}
.cinza {
	color: #C0C0C0;
}
-->
</style>
  <link href="css/style.css" rel="stylesheet" type="text/css" />
<form method="post" onSubmit="disabledBttn(this)" action="<?=$_SERVER[PHP_SELF]."?".$_SERVER[QUERY_STRING]?>">
	<table background="imgs/fundo_textura1.gif" width="716" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td width="716"><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
                  <tr>
                    <td colspan="2" align="center" bgcolor="#000000"><b><font color="#FFFFFF">Busca de ID por nick </font></b></td>
                  </tr>
<tr>
  <td width="35%" align="right"><strong><font color="#000000">Nick do Personagem </font></strong></td>
  <td width="65%">                      <input type="text" name="id" /> </td>
                </tr>
                <tr>
                  <td colspan="2" align="center"><input name="submit" type="submit" class="button6" value="Procurar" />
                  <input name="acao" type="hidden" id="acao" value="search" /></td>
                  </tr>
<tr>
                  <td colspan="2" align="center"><div align="left">
                    <p><strong>Instru&ccedil;&otilde;es de como Utilizar o Sistema de search ID<br>
                    </strong>1 Passo - Localize o campo branco acima.<br> 
                    2 Passo - No campo em branco coloque o nick do player que voc&ecirc; deseja saber a ID. <br>3 Passo - Clique em Procurar.</p>
                    <p>Apos seguir os 3 passos acima, e clicar em procurar vai abrir uma janela falando assim.<BR>
                     Informa&ccedil;&atilde;o da ID do Player, abaixo grifado estara a id do player, caso n&atilde;o aparece nada este nick n&atilde;o foi localizado no banco de dados.</p>
                  </div></td>
                  </tr>

                  <tr>
                  </tr>
                </table></td>
              </tr>
  </table>
</form>
<?php
}
else
{
$procura = $_POST['id'];

if (anti_sql($procura) != 0) {
echo "<center><font face=verdana size=2>Uso de Caracteres não Permitidos!</a>";
} else {

function subDiretorio($pasta)
{

	$total = 0;
	for($i=0;$i<strlen($pasta);$i++)
	{			
		$total += ord(strtoupper($pasta[$i]));
			if($total >= 256)
			{
				$total = $total - 256;
			}
		
	}
	return $total;
}


$procuraPlayer = ''.$rootDir.'DataServer/userdata/'.subDiretorio($procura).'/'.$procura.'.dat';
if(file_exists($procuraPlayer))
{

	// lê o arquivo para descobrir a ID
	$aberto = fopen($procuraPlayer, "r");
	$conteudoDat =fread($aberto,filesize($procuraPlayer));
	@fclose($aberto);

	$PlayerID = trim(substr($conteudoDat,0x2c0,10),"\x00");

	}
	?>
<table width="32%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#330099">
<tr>
<td width="7%" align="center" bgcolor="#000000"><font color="#FFFFFF" class="branco"><strong>Informa&ccedil;&atilde;o  da ID do Player</strong></font></td>

</tr>
<tr>
<td align="center"><span class="vermelho"><strong><? echo $PlayerID; ?></strong></span>&nbsp;</td>
</tr>
</table>
<table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
  <tr>
    <td colspan="2" align="center"><div align="left">
      <p align="center">Ol&aacute; Game Master a id do Player foi encontrada &eacute; ( <strong><? echo $PlayerID; ?></strong> ) <BR>
        Caso n&atilde;o aparece nada entre os parenteses acima o nick citado n&atilde;o foi localizado no banco de dados portanto nick n&atilde;o existe ouo char foi deletado.<strong><br />
        </strong></p>
</div></td>
  </tr>
</table>
<p>
  <?
}}
?>
</p>