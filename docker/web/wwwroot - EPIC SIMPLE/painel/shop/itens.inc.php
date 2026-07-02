<meta http-equiv="Content-type" content="text/html; charset=ISO-8859-1" />
<script language="JavaScript">
function abrir(URL) {

  var width = 300;
  var height = 415;

  var left = 99;
  var top = 99;

  window.open(URL,'janela', 'width='+width+', height='+height+', top='+top+', left='+left+', scrollbars=yes, status=no, toolbar=no, location=no, directories=no, menubar=no, resizable=no, fullscreen=no');

}
</script>

<?
/*----------------------------------------------------------------------
Painel Summit Tale
----------------------------------------------------------------------*/
if (Painel!=1) exit;

$username=$_SESSION["ID"];
$pasta = $func->numDir($qCharID);

session_unregister("enviado");

//Pasta de Controle de Pontos Bonus
$arquivo_bonus = "shop/bonusplayer/$username.arc";
$pega_bonus_zero = "shop/bonusplayer/bonus_zero.arc";
//Verifica se pasta de Bonus Existe
if (!file_exists($arquivo_bonus)) {
copy($pega_bonus_zero, $arquivo_bonus);
}


//Pega os Bônus do Player
$abreB = fopen($arquivo_bonus, "r");
$saldoB = fread($abreB, filesize($arquivo_bonus));
fclose($abreB);

?>
<link href="css/style.css" rel="stylesheet" type="text/css">
</div>
<table width="100%" border="0" align="center" cellpadding="6" cellspacing="0">
   <td align="center" bgcolor="#ffcc00"><strong><font size="2" color="black">Loja(<? echo $nomecat; ?>)</font></strong></td></table>
   <table width="80%" border="0" align="center" cellpadding="6" cellspacing="0">
  <tr>
    <td> 
      <table width="100%" border="0" cellpadding="0" cellspacing="0">
        <tr>
          <td class="btn btn-action"><b><font color="#009900"><img src="imgs/moedas.gif" width="25" height="16" />Créditos:</font></b> <font color="black"> <? echo $saldoB; ?> Cr&eacute;ditos</font></td>
          

          <div></div>
          <td align="right"><a class="btn btn-action" href="?sess=extrato" role="button"><font color="#000000">Extrato</font></a></td>
        </tr>
      </table>
      <font size="2" face="Verdana, Arial, Helvetica, sans-serif"><br>
      </font>
      <font size="2" face="Verdana, Arial, Helvetica, sans-serif"><br>
      </font>
      <table width="100%" border=0 cellpadding="2" cellspacing="2">
<tr>
<?php
//Verifica se arquivo existe
if (file_exists($arquivo)) {

//ABRE O ARQUIVO
$ponteiro = fopen($arquivo, "r");
//LÊ
$conteudo = fread($ponteiro, filesize($arquivo) );
//FECHA O ARQUIVO
fclose($ponteiro);
//EXPLODE AS LINHAS QUANDO PULAR LINHA
	$linha = explode("\r", $conteudo);

if(!isset($colunas)) $colunas = 2;
$y=1;

	for($i = 0; $i < sizeof($linha); $i++) {


 //SEPARANDO OS DADOS POR ; (PONTO E VIRGULA)
 $parte = explode(";", $linha[$i]);

 $codigo_item = trim("$parte[0]");
 $nome_item =  trim("$parte[1]");
 $level_item =  trim("$parte[2]");
 $valor_item =  trim("$parte[3]");
 $spec_item =  trim("$parte[4]");
 $tipo_item =  trim("$parte[5]");
 $peso_item =  trim("$parte[6]");

 $resto = $y % $colunas;
?>
<td align="center" valign="top"><table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr>
    <td align="center" height="25" bgcolor="#333333"><font size="2" face="Verdana, Arial, Helvetica, sans-serif" color="#FFFFFF"><b><? echo $nome_item; ?> </b> </font></td>
  </tr>

  <tr>
    <td align="center">
      <table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr>
    <td align="center">
      <table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr>
    <td height="100" align="center" bgcolor="#000000"><?php
$verifica_imagem[$i] = "shop/imgitens/it$codigo_item.bmp";
if (file_exists($verifica_imagem[$i])) {
echo "<img src='shop/imgitens/it$codigo_item.bmp'>";

$criatudo="shop/infoitens/$codigo_item.php";
$filebase="shop/infoitens/0base.php";
if (!file_exists($criatudo)) {
copy($filebase, $criatudo);
}

include ("shop/infoitens/$codigo_item.php"); ?>
         <td align="left" bgcolor="#000000"><?php echo $showstatus; ?></td> <?php
} else {
echo "<img src='/painel/imgs/loading.gif' width='20' height='20'/>";
}
?>      <br></td>
    </tr>
</table>
  </tr>
</table>
      <font size="2" face="Verdana, Arial, Helvetica, sans-serif">
      <?
      $http = $_SERVER['PHP_SELF'];
	  ?>
      </font></tr>
  <tr>
    <td><table width="100%" border="0" cellspacing="0" cellpadding="4">
    <form name="form1" method="post" action="<? echo $_SERVER[PHP_SELF]; ?>?sess=compra">
      <tr>
        <td><div align="left"><font size="2" face="Verdana, Arial, Helvetica, sans-serif"><b><img src="imgs/setinha.gif" width="11" height="11"></b>
              <select name="personagem" id="personagem">
                <option value="" selected>Selecione o Personagem</option>
                <?
            $qCharID=($_SESSION["charID"])?$_SESSION["charID"]:$_SESSION["ID"];
            $charInfo=$dirUserInfo . ($func->numDir($qCharID)) . "/" . $qCharID . ".dat";

            if(file_exists($charInfo) && ( filesize($charInfo)==240) )
            {
                $fRead=false;
                $fOpen = fopen($charInfo, "r");
                $fRead =fread($fOpen,filesize($charInfo));
                @fclose($fOpen);

                // list char information
                $charNameArr=array(
                    "48" => trim(substr($fRead,0x30,15),"\x00"),
                    "80" => trim(substr($fRead,0x50,15),"\x00"),
                    "112"=> trim(substr($fRead,0x70,15),"\x00"),
                    "144"=> trim(substr($fRead,0x90,15),"\x00"),
                    "176"=> trim(substr($fRead,0xb0,15),"\x00"),
                );

                if(count($charNameArr)>0)
                {
                    foreach($charNameArr as $key=>$value)
                    {
                        $expValue=explode("\x00",$value);
                        if($expValue[0]!=""){ echo "<option >".$expValue[0]."</option>"; }
                    }
                }
                else
                {
                    echo "Vazio";
                }

            }
            else
            {
                echo "Vazio";
            }

?>
              </select>
        </font></div></td>
      </tr>
      <tr>
        <td><div align="left"><font size="2" face="Verdana, Arial, Helvetica, sans-serif"><img src="imgs/setinha.gif" width="11" height="11">       
              <? include ("shop/spec.inc.php"); ?>
        </font></div></td>
      </tr>
	  <tr>
        <td><div align="left"><font size="2" face="Verdana, Arial, Helvetica, sans-serif" color="#FFFFFF"><img src="imgs/setinha.gif" width="11" height="11">
             <select name="quantidade" id="quantidade">
             <option value="0" selected="selected">Selecione a Quantidade</option>
			 <option value="1">1</option>
             <option value="2">2</option>
             <option value="3">3</option>
             <option value="4">4</option>
             <option value="5">5</option>
             <option value="6">6</option>
             <option value="7">7</option>
             <option value="8">8</option>
             <option value="9">9</option>
             <option value="10">10</option>
            </select>
           </div></td>
      </tr>
      <tr>
        <td align="center"><div align="center"><font size="2" face="Verdana, Arial, Helvetica, sans-serif" color="#000"><strong><br />
          Level: <? echo $level_item; ?></strong>
              <?
			if ($valor_item > $saldoB) {
		  ?>
          </font>
            <br />
            <br />
            <table width="100%" border="0" cellspacing="0" cellpadding="0">
            <tr>
              <td height="30" align="center" bgcolor="#990000"><font size="2" face="Verdana, Arial, Helvetica, sans-serif"><b><font color="#FFFFFF"><br />
                <img src="imgs/alerta.png" width="30" height="30" align="top" /></font></b></font><br />
                <font color="#FFFFFF" size="2" face="Verdana, Arial, Helvetica, sans-serif"><b>Você não
                    possui créditos suficiente!</b></font></td>
              </tr>
          </table>
            <br />
            <font size="2" face="Verdana, Arial, Helvetica, sans-serif">
          <? } else { ?>
          
          <input name="nome" type="hidden" id="nome" value="<? echo $nome_item; ?>">
          <input name="valor" type="hidden" id="valor" value="<? echo $valor_item; ?>">
          <input name="acao" type="hidden" id="acao" value="Comprar">
          <input name="coditem" type="hidden" id="coditem" value="<? echo $codigo_item; ?>">
          <input name="arquivo" type="hidden" id="coditem" value="<? echo $arquivo; ?>">
          <br><? if($valor_item == 0){ echo "<br/>Não Disponivel Para Compra <br/>"; }else{ ?>
          <input type="image" style="border-width:0;"  name="imageField" id="imageField" src="imgs/bt_comprar.png" onClick="cf=confirm('Confirma que deseja enviar este item?');if(!cf){return false}">
          <? } }?>
          <br>
          <b><font color="#009900">Valor:</font></b> <font color="#009900"><? echo $valor_item; ?> Créditos</font></font></div></td>
      </tr>
    </form>
    </table>        </td>
  </tr>
</table>   </td>
<?
        if( $resto == 0) print "\n</tr>\n<tr>\n";

        $y++;

    }

             if( $resto != 0) print "\n</tr>";
}
else
{
}
?>
</table></td>
  </tr>
  <tr>
    <td><font size="2" face="Verdana, Arial, Helvetica, sans-serif">&nbsp;</font></td>
  </tr>
</table>
