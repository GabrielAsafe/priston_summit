<?
if (PT!=1)  exit;
?>

<table background="imgs/fundo_textura1.gif" width="716" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td width="716"><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
                  <tr>
                    <td colspan="2" align="center" bgcolor="#000000"><b><font color="#FFFFFF">Restaurar char deletado </font></b></td>
                  </tr>
<table width="795" border="0">
  <tr>
    <td width="789"><form id="form1" name="form1" method="post" action="">
      <table width="652" height="69" border="0" align="center">
        <tr>
          <td width="102">Nick do Personagem</td>
          <td width="540"><label>
            <input type="text" name="nick" id="nick" />
            </label></td>
        </tr>
        <tr>
          <td>&nbsp;</td>
          <td><label>
            <input type="submit" name="button" id="button" value="Restaurar char" />
            <input name="acao" type="hidden" id="acao" value="restaurar" />
          </label></td>
        </tr>
        <tr>
                  <td colspan="2" align="center"><div align="left">
                    <p><strong>Instru&ccedil;&otilde;es de como usar o sistema de restaurar char deletados<br>
                    </strong>1 Passo - Localize o campo branco acima.<br> 
                    2 Passo - No campo em branco coloque o nick do player que voc&ecirc; deseja restaura o char.<br>
                    3 Passo - Clique em restaurar char.</p>
                    <p>Apos seguir os 3 passos acima, e clicar em restaurar vai abrir abrir uma mensagem falando.<BR>
                    qual a&ccedil;&atilde;o que deu, se foi restaurado com sucesso, se o char n&atilde;o existe etc...</p>
                    </div>
                  </div></td>
                  </tr>
      </table>
        </form>
    </td>
  </tr>
</table>
<table width="300" border="0">
  <tr>
    <td><?
	if($_POST[acao] == "restaurar"){
	$nick = $_POST['nick'];
	
	
	
	
	$charRecover=trim($func->char_filter(trim($nick)),"\x00");

                if(!$func->is_valid_string($charRecover))
                {

                    $charDatDelete = $dirUserDelete . "/" . $charRecover . ".dat";

                    if(file_exists($charDatDelete))
                    {

                        $fRead=false;
                        $fOpen = fopen($charDatDelete, "r");
                        $fRead =fread($fOpen,filesize($charDatDelete));
                        @fclose($fOpen);

                        // details
                        $charID = trim(substr($fRead,0x2c0,10),"\x00");
                        $charName = trim(substr($fRead,0x10,15),"\x00");

                        

                            $charInfo=$dirUserInfo . ($func->numDir($charID)) . "/" . $charID . ".dat";

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

                            $chkEmpArr=array();
                            $chkChar=array();
                            foreach($charNameArr as $key=>$value)
                            {
                                if(empty($value)) $chkEmpArr[]=$key;
                                else $chkChar[]=$key;
                            }

                            if(count($chkChar)<5)
                            {

                                // point to each information line
                                $startPoint=$chkEmpArr[0];
                                $endPoint=$startPoint+15;

                                 // Write info-----------------------------------------------------------------------
                                $fRead=false;
                                $fOpen = fopen($charInfo, "r");
                                while (!feof($fOpen)) {
                                @$fRead = "$fRead" . fread($fOpen, filesize($charInfo) );
                                }
                                fclose($fOpen);

                                // Fill in 00 to left character
                                $addOnLeft=false;
                                $leftLen=15-strlen($charName);
                                for($i=0;$i<$leftLen;$i++)
                                {
                                    $addOnLeft.=pack("h*",00);
                                }
                                $writeName=$charName.$addOnLeft;

                                $sourceStr = substr($fRead, 0, $startPoint) . $writeName . substr($fRead, $endPoint);
                                $fOpen = fopen($charInfo, "wb"); 
                                fwrite($fOpen, $sourceStr, strlen($sourceStr));
                                fclose($fOpen);

                                copy($dirUserDelete . "/" . $charName . ".dat", $dirUserData . ($func->numDir($charName)) . "/" .$charName . ".dat");
                                unlink($dirUserDelete . "/" . $charName . ".dat");

                                echo "<b><center><font color='green'>O char <u>". $charName ."</u> foi restaurado com sucesso!";

                            }
                            else
                            {
                                echo "<b><center><font color='red'>Conta está cheia,impossivel restaurar";
                            }

                        
                        

                    }
                    else
                    {
                        echo "<b><center><font color='red'>o char digitado não foi encontrado ou não existe!";
                    }

                }
                else
                {
                    echo "<b><center><font color='green'>Por Favor escreva outro nick,sem caracteres especiais!";
                }

                
	
	
	
	
	
	}
	
	
	?></td>
    </table>
  </tr>
</table>
</body>
</html>
