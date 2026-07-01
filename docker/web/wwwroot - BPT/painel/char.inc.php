<?if (Painel!=1) exit;?>
<?
require_once "incluir/configura.php";
function RandomPass($numchar){
   $letras = "A,B,C,D,E,F,G,H,I,J,K,1,2,3,4,5,6,7,8,9,0";
   $array = explode(",", $letras);
   shuffle($array);
   $senha = implode($array, "");
   return substr($senha, 0, $numchar);
}
$pgid = RandomPass(5);

            $qCharID=($_SESSION["charID"])?$_SESSION["charID"]:$_SESSION["ID"];

            if($_POST[action]!="Criar Char")
            {
?>
<link rel="stylesheet" type="text/css" href="css/main.css">
<link rel="stylesheet" type="text/css" href="css/char.css">
    </div>
    <table width="100%" border="0" align="center" cellpadding="2" cellspacing="2">
                          <tr>
                             <td colspan="2" bgcolor="#ffcc00" height="25"><div align="center"><strong><font color="#000000">Criar Personagem </font></strong>
                            </td>
                            </tr></table>
            <form method="post" onSubmit="disabledBttn(this)" action="<?=$_SERVER[PHP_SELF]."?".$_SERVER[QUERY_STRING]?>&pgid=$pgid">
                        <div align="center" style="margin-top: 20px;"><img src="imgs/classes/Tempskrons.png"></div>
                        <br>
                            <div align="center" name="class">

                                    <input name="class" type="radio" id="Archer" value="Archer">
                                    <label for="Archer"><img alt="ImgArqueira.png" src="imgs/classes/ImgArqueira.png">Arqueira</label>
                                    <input name="class" type="radio" id="fs" value="Fighter">
                                    <label for="fs"><img alt="ImgLutador.png" src="imgs/classes/ImgLutador.png">Lutador</label>
                                    <input name="class" type="radio" id="ps" value="Pikeman">
                                    <label for="ps"><img alt="ImgPike.png" src="imgs/classes/ImgPike.png">Pikeman</label>
                                    <input name="class" type="radio" id="ms" value="Mechanician">
                                    <label for="ms"><img alt="ImgMech.png" src="imgs/classes/ImgMech.png">Mecânico</label>

                            </div>

                        <br>

                        <div align="center"><img src="imgs/classes/Morions.png"></div>
                        <br>
                            <div align="center">
                                    <input name="class" type="radio" id="pris" value="Priestess">
                                    <label for="pris"><img alt="ImgSacer.png" src="imgs/classes/ImgSacer.png">Sacerdotisa</label>
                                    <input name="class" type="radio" id="ks" value="Knight">
                                    <label for="ks"><img alt="ImgCavaleiro.png" src="imgs/classes/ImgCavaleiro.png">Cavaleiro</label>
                                    <input name="class" type="radio" id="mg" value="Magician">
                                    <label for="mg"><img alt="ImgMago.png" src="imgs/classes/ImgMago.png">Mago</label>
                                    <input name="class" type="radio" id="ata" value="Atalanta">
                                    <label for="ata"><img alt="ImgAtalanta.png" src="imgs/classes/ImgAtalanta.png">Atalanta</label>

                            </div>

                    <br>
                    <div align="center">                 
                        <td>Nick: <input name="newchar" type="text" maxlength="15">
                        <input type="submit" class="button4" value="Criar Char"></td>
                        <input type="hidden" name="action" value="Criar Char">
                    </div>
                    <hr>
            </form>

            <?

            }
            else
            {
            include_once("gravarchar.php");
                
                // Fill in 00 to left character
                $leftLen=10-strlen($qCharID);
                for($i=0;$i<$leftLen;$i++)
                {
                    $addOnLeft.=pack("h*",00);
                }
                $writeAccName=$qCharID.$addOnLeft;

                $charInfo=$dirUserInfo . ($func->numDir($qCharID)) . "/" . $qCharID . ".dat";

                if(!file_exists($charInfo))
                {
                    copy("criarchars/info.dat",$dirUserInfo . ($func->numDir($qCharID)) . "/" . $qCharID. ".dat");

                    $fRead=false;
                    $fOpen = fopen($charInfo, "r");
                    while (!feof($fOpen)) {
                    @$fRead = "$fRead" . fread($fOpen, filesize($charInfo) );
                    }
                    fclose($fOpen);

                    // Change character class ----------------------------------------------------------------
                    $sourceStr = substr($fRead, 0, 16) . $writeAccName . substr($fRead, 26);
                    $fOpen = fopen($charInfo, "wb");
                    fwrite($fOpen, $sourceStr, strlen($sourceStr));
                    fclose($fOpen);

                    echo "<br><center><font color=#CC0000><b>Aviso:</b></font><br><br>- Arquivo de ID Criado, AGORA CRIE SEU CHAR!<br><br><br>";
                }
                else
                {
                    if( filesize($charInfo)==240)
                    {
                        $newName=trim($func->char_filter(trim($_POST["newchar"])),"\x00");

                        //Limpando Caracteres de acentos
                        function strace($a)
                        {
                        $a = eregi_replace("[àáâäã]","a",$a);
                        $a = eregi_replace("[èéêë]","e",$a);
                        $a = eregi_replace("[ìíîï]","i",$a);
                        $a = eregi_replace("[òóôöõ]","o",$a);
                        $a = eregi_replace("[ùúûü]","u",$a);
                        $a = eregi_replace("[ÀÁÂÄÃ]","A",$a);
                        $a = eregi_replace("[ÈÉÊË]","E",$a);
                        $a = eregi_replace("[ÌÍÎÏ]","I",$a);
                        $a = eregi_replace("[ÒÓÔÖÕ]","O",$a);
                        $a = eregi_replace("[ÙÚÛÜ]","U",$a);
                        $a = eregi_replace("ç","c",$a);
                        $a = eregi_replace("Ç","C",$a);
                        $a = eregi_replace("ñ","n",$a);
                        $a = eregi_replace("Ñ","N",$a);
                        $a = str_replace("´","",$a);
                        $a = str_replace("`","",$a);
                        $a = str_replace("¨","",$a);
                        $a = str_replace("^","",$a);
                        $a = str_replace("~","",$a);
                        return $a;
                        }
                        $newName = strace("$newName");

                        if(!$func->is_valid_string($newName))
                        {

                            $charDat = $dirUserData . ($func->numDir($newName)) . "/" . $newName . ".dat";

                            if(!file_exists($charDat))
                            {
                                copy("criarchars/char.dat",$dirUserData . ($func->numDir($newName)) . "/" . $newName. ".dat");

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
                                    $leftLen=15-strlen($newName);
                                    for($i=0;$i<$leftLen;$i++)
                                    {
                                        $addOnLeft.=pack("h*",00);
                                    }
                                    $writeName=$newName.$addOnLeft;


                                    $sourceStr = substr($fRead, 0, $startPoint) . $writeName . substr($fRead, $endPoint);
                                    $fOpen = fopen($charInfo, "wb");
                                    fwrite($fOpen, $sourceStr, strlen($sourceStr));
                                    fclose($fOpen);

                                    // Write data-------------------------------------------------------------------------
                                    $fRead=false;
                                    $fOpen = fopen($charDat, "r");
                                    while (!feof($fOpen)) {
                                    @$fRead = "$fRead" . fread($fOpen, filesize($charDat) );
                                    }
                                    fclose($fOpen);

                                    $bin = $func->char2Num($_POST["class"]);
                                    $bina= pack("h*",$bin);

                                    // Change character class ----------------------------------------------------------------
                                    $sourceStr = substr($fRead, 0, 16) . $writeName . substr($fRead, 31, 17) . ($func->charResetHair($_POST['class'], 1)) . substr($fRead, 69, 43) . ($func->charResetHair($_POST['class'], 2)) . substr($fRead, 136, 60) . $bina . substr($fRead, 197, 7) . ($func->charResetState($_POST['class'])) . substr($fRead, 224, 284) . ($func->charResetSkill()) . substr($fRead, 524, 0) . ($func->charResetMastery()) . substr($fRead, 556, 148) . $writeAccName . substr($fRead, 714);
                                    $fOpen = fopen($charDat, "wb");
                                    fwrite($fOpen, $sourceStr, strlen($sourceStr));
                                    fclose($fOpen);


                           echo "<center><font color=#CC0000><b>Aviso:</b></font><br><br>Personagem criado com sucesso, level 100 com 4kk<br><br>";
                                }
                                else
                                {
                                    echo "<center><font color=#CC0000><b>Aviso:</b></font><br><br>Você so pode ter até 5 personagens em cada CONTA!<br><br>";
                                }
                            }
                            else
                            {
                                echo "<center><font color=#CC0000><b>Aviso:</b></font><br><br>Este nome já está sendo utilizado por outro jogador. Volte e escolha outro.<br><br>";
                            }

                        }
                        else
                        {
                            echo "<center><font color=#CC0000><b>Aviso:</b></font><br><br>O Nick que você escolheu não pode ser utilizado desculpe, volte e tente novamente!<br><br>";
                        }


                    }
                    else
                    {
                        echo "<center><font color=#CC0000><b>Aviso:</b></font><br><br>Arquivo CORROMPIDO<br><br>";
                    }
                }

                echo "<center><font color='black'><strong><a href=\"$_SERVER[PHP_SELF]?$_SERVER[QUERY_STRING]&pgid=$pgid\">VOLTAR</strong></font></a>";

            }




//------------------------------------------------------------ DELETE CHARACTER