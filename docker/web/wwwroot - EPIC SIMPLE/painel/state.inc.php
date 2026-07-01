<? if (Painel!=1) exit;
include_once "injection.php";
?>
<?
/*----------------------------------------------------------------------
Painel Player Versão 0.4
Desenvolvidor Por: Mak (sirmakloud@gmail.com)
Editado Por: Jiraya (richard.cva21@hotmail.com(
----------------------------------------------------------------------*/
        if($_SESSION["charLevel"]<100)
        {
           echo "<table width=440 border=0 cellspacing=0 cellpadding=0>
                 <tr>
                 <td height=100 align=center><b><font color=#FF0000><div align=\"center\"><img src=\"imgs/alerta.png\"><br><br>É obrigatório que seu personagem tenha mais que level 100 para
ter acesso ao nosso sistema de alteração e distribuição de skills do painel de controle.<br></font><BR> Volte para usar esta opção quando o seu personagem estiver no level 100.</b></td>
                 </tr>
                 </table>";
        }
		else
        {
            $fOpen = fopen($_SESSION["charDir"], "r");
            $fRead =fread($fOpen,4096);

            @fclose($fOpen);

            // details
            $str1 = bin2hex(substr($fRead,0xcc,1));
            $str2 = bin2hex(substr($fRead,0xcd,1));
            $strength = hexdec("$str2"."$str1");

            $spi1 = bin2hex(substr($fRead,0xd0,1));
            $spi2 = bin2hex(substr($fRead,0xd1,1));
            $spirit = hexdec("$spi2"."$spi1");

            $tal1 = bin2hex(substr($fRead,0xd4,1));
            $tal2 = bin2hex(substr($fRead,0xd5,1));
            $talent = hexdec("$tal2"."$tal1");

            $agi1 = bin2hex(substr($fRead,0xd8,1));
            $agi2 = bin2hex(substr($fRead,0xd9,1));
            $agility = hexdec("$agi2"."$agi1");

            $hea1 = bin2hex(substr($fRead,0xdc,1));
            $hea2 = bin2hex(substr($fRead,0xdd,1));
            $health = hexdec("$hea2"."$hea1");

            $defaultP=$func->checkStatePoints($_SESSION["charLevel"]);

            $totalP=($strength+$spirit+$talent+$agility+$health)-99;

            if(anti_sqli($_POST[action]!="state"))
            {
?>  

            </div>
            <hr>
            <form method="post" onSubmit="disabledBttn(this)" action="<?=$_SERVER[PHP_SELF]."?".$_SERVER[QUERY_STRING]?>">
            <table width="100%" border="0" align="center" cellpadding="2" cellspacing="2">
              <tr>
                <td><b><img src="imgs/setinha.gif" width="11" height="11"><strong> GERENCIAMENTO DOS PONTOS DE STATS</strong></b></td>
              </tr>
              <tr>
                <td><table class="padding_all" width="100%" border="0" cellpadding="0" cellspacing="0">
                    <tbody>
                      <tr>
                        <td width="20%" align="center" bgcolor="#ffcc00"><strong><font color="black">Força </font></strong></td>
                        <td width="20%" align="center" bgcolor="#ffcc00"><strong><font color="black">Inteligência </font></strong></td>
                        <td width="20%" align="center" bgcolor="#ffcc00"><strong><font color="black">Talento </font></strong></td>
                        <td width="20%" align="center" bgcolor="#ffcc00"><strong><font color="black">Agilidade </font></strong></td>
                        <td width="20%" align="center" bgcolor="#ffcc00"><strong><font color="black">Vitalidade </font></strong></td>
                      </tr>
                      <tr>
                        <td align="center" bgcolor="#e5e5e5"><input type="text" value="<?=$strength?>" name="strength" size="3" maxlength="4">
                        </td>
                        <td align="center" bgcolor="#e5e5e5"><input type="text" value="<?=$spirit?>" name="spirit" size="3" maxlength="4">
                        </td>
                        <td align="center" bgcolor="#e5e5e5"><input type="text" value="<?=$talent?>" name="talent" size="3" maxlength="4">
                        </td>
                        <td align="center" bgcolor="#e5e5e5"><input type="text" value="<?=$agility?>" name="agility" size="3" maxlength="4">
                        </td>
                        <td align="center" bgcolor="#e5e5e5"><input type="text" value="<?=$health?>" name="health" size="3" maxlength="4">
                        </td>
                      </tr>
                    </tbody>
                  </table>
                </td>
              </tr>
              <tr>
                <td align="right">Pontos disponíveis: <font size="3">
                  <?=$defaultP-$totalP?>
                </font></td>
              </tr>
              <tr>
                <td align="center">
                  <input class="btn btn-action btn-char top-margin" value="Redistribuir" class="button" type="submit">
                </td>
              </tr>
            </table>
            <input type="hidden" name="action" value="state">

            </form>

<?
            }
            else
            {
                $checkSubmitP=(anti_sqli($_POST['strength'])+anti_sqli($_POST['spirit'])+anti_sqli($_POST['talent'])+anti_sqli($_POST['agility'])+anti_sqli($_POST['health']))-99;

                if( ($checkSubmitP>$defaultP))
                {
                    echo "<b><div align=\"center\">Você utilizou (". $checkSubmitP .") Pontos de Stats, sendo que você possui (". $defaultP .") Stats!<br> Redistribua seus stats conforme a quantidade que você possui! </div><br><br>";
                }
                else
                {
                    $charStr=pack('i', anti_sqli($_POST['strength']));
                    $charSpi=pack('i', anti_sqli($_POST['spirit']));
                    $charTal=pack('i', anti_sqli($_POST['talent']));
                    $charAgi=pack('i', anti_sqli($_POST['agility']));
                    $charHea=pack('i', anti_sqli($_POST['health']));
                    $charStateStr=$charStr . $charSpi . $charTal . $charAgi . $charHea;

                    $fRead=false;
                    $fOpen = fopen($_SESSION["charDir"], "r");
                    while (!feof($fOpen)) {
                    @$fRead = "$fRead" . fread($fOpen, filesize($_SESSION["charDir"]) );
                    }
                    fclose($fOpen);

                    $sourceStr = substr($fRead, 0, 204) . $charStateStr . substr($fRead, 224);
                    $fOpen = fopen($_SESSION["charDir"], "wb"); 
                    fwrite($fOpen, $sourceStr, strlen($sourceStr));
                    fclose($fOpen);

                   echo "<b>
	 <div align=\"center\"><font color='Green'>Pontos de Status Atualizados com SUCESSO!</font></div><br><br>
";
                }

               echo "<b>
	 <div align=\"center\"> <a href=\"$_SERVER[PHP_SELF]?$_SERVER[QUERY_STRING]\">Voltar</a> </div><br><br>
";
            }
			
			}
			
