          <?
          include_once "injection.php";

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
            ?>
                    
                    <?
            $nomedochar = $_SESSION["ID"];

            //Vendo se o Player est&aacute; Banido
            $connection = odbc_connect( $connection_string, $user, $pass );
            $query_ban = "SELECT * FROM [accountdb].[dbo].[".( strtoupper($nomedochar[0]) ) ."GameUser] WHERE [userid]='$nomedochar'";
            $q_ban = odbc_exec($connection, $query_ban);
            $qt_ban = odbc_do($connection, $query_ban);
            while(odbc_fetch_row($qt_ban)) {
            $verbanido = odbc_fetch_array($q_ban);
            $taban=$verbanido[BlockChk];
            }
            if ($taban == "1") {
            echo "<meta HTTP-EQUIV='Refresh' CONTENT='0;URL=banido.php'>";
            }

            $connection = odbc_connect( $connection_string, $user, $pass );
            $queryVPun = "SELECT * FROM [ADM].[dbo].[LogsBan] WHERE [idplayer]='$nomedochar' ORDER BY data desc";
            $qVPun = odbc_exec($connection, $queryVPun);
            $qtVPun = odbc_do($connection, $queryVPun);
            $VPun = 0;
            while(odbc_fetch_row($qtVPun)) $VPun++;
            if($VPun>0)
            {
            $PunidoC++;
            }

            if ($PunidoC) {
            ?>
                                    <tr>
                                      <td align="center" bgcolor="#540305"><font color="#FFFFFF"><strong>VOC&Ecirc; FOI PUNIDO NO SERVER PELO(S) MOTIVO(S) ABAIXO:</strong></font></td>
                                    </tr>
                                    <?
            $nomedochar = $_SESSION["ID"];


            $connection = odbc_connect( $connection_string, $user, $pass );
            $queryPUN = "SELECT TOP 5 * FROM [ADM].[dbo].[LogsBan] WHERE [idplayer]='$nomedochar' ORDER BY data desc";
            $qPUN = odbc_exec($connection, $queryPUN);
            $qtPUN = odbc_do($connection, $queryPUN);
            $contapunicao = 0;
            while(odbc_fetch_row($qtPUN)) {
            $i++;
            $farr = odbc_fetch_array($qPUN);
            $motivo=$farr[motivo];
            $punicao=$farr[punicao];
            $diaban=$farr[diaban];
            $mesban=$farr[mesban];
            $anoban=$farr[anoban];
            $diadesban=$farr[diadesban];
            $mesdesban=$farr[mesdesban];
            $anodesban=$farr[anodesban];


            ?>
                                    <tr>
                                      <td bgcolor="#FFFFFF"><img src="imgs/ico_alert.gif" width="14" height="14" align="absmiddle"> Recebeu <font color="#FF0000"><strong><? echo $punicao; ?></strong></font> - <strong>Motivo</strong>: <? echo $motivo; ?> - <strong>Data</strong>:&nbsp;<? echo "$diaban/$mesban/$anoban"; ?></td>
                                    </tr>
                                    <? } ?>
                                  </table>
                                  <? } ?>
                        </table></td>
                    </tr>
                    <tr>
                      <td style="border-bottom: solid 1px #cecece; border-top: solid 1px #cecece; border-left: solid 1px #cecece; border-right: solid 1px #cecece" class="padding_all"><p align="center">
                        <?
            $nomedochar = $_SESSION["ID"];
            //$nomedochar = $_SESSION["charID"];
            $connection = odbc_connect( $connection_string, $user, $pass );
            $query = "SELECT * FROM [ClanDb].[dbo].[CL]  WHERE UserID = '$nomedochar' ";
            $qLider = odbc_exec($connection, $query);
            $qtLider = odbc_do($connection, $query);

            $i = 0;
                            while(odbc_fetch_row($qtLider)){
                            $nomeclan=odbc_result($qtLider,2);

                             $i++;

                            if($i==0) {} else {
            ?>
                        <img src="imgs/setinha.gif" width="11" height="11"> Painel de Administra&ccedil;&atilde;o
                        de Clan </p>
                        <table width="100%" border="0" cellspacing="0" cellpadding="3">
                          <tr>
                            <td width="131" align="center"><a href="?sess=admclan"><img src="imgs/admclan.gif" width="112" height="22" border="0"></a></td>
                            <td width="131" align="center"><a href="?sess=membros"><img src="imgs/membrosclan.gif" width="112" height="22" border="0"></a></td>
                            </tr>
                          </table>
                        <?
            }}
            $CHAR=($_GET["char"])?$_GET["char"]:$_POST["char"];

            if( !$CHAR )
            {
              $query = "SELECT * FROM [clanDB].[dbo].[CL]";
              $go = odbc_do($connection,$query);
            function AcertarLinhasODBC($conexao,$query){
              $resultado = odbc_exec($conexao,$query);
              $contador=0;
              while($temp = odbc_fetch_into($resultado, &$counter)){
                $contador++;
              }
              return $contador;
            }
              $online = AcertarLinhasODBC($connection,$query);
            ?>
                        
            <?
            }
            else
            {

                $charDat = $dirUserData . ( $func->numDir($CHAR) ) . "/" . $CHAR . ".dat";

                if(file_exists($charDat) && ( (filesize($charDat)==16384) || (filesize($charDat)==111376) || (filesize($charDat)==220976) ) )
                {

                    $fOpen = fopen($charDat, "r");
                    $fRead =fread($fOpen,filesize($charDat));
                    @fclose($fOpen);

                    // details
                    $charLevel = substr($fRead,0xc8,1);
                    $charClass = substr($fRead,0xc4,1);
                    $charName = trim(substr($fRead,0x10,15),"\x00");
                    $charID = trim(substr($fRead,0x2c0,10),"\x00");

                    if( (strtolower($charID)==strtolower($_SESSION["ID"]))  )

                    {

                        if($CHAR==$charName)
                        {

                            switch (ord($charClass))
                            {
                                case 1: $class = 'Fighter'; break;
                                case 2: $class = 'Mechanician'; break;
                                case 3: $class = 'Archer'; break;
                                case 4: $class = 'Pikeman'; break;
                                case 5: $class = 'Atalanta'; break;
                                case 6: $class = 'Knight'; break;
                                case 7: $class = 'Magician'; break;
                                case 8: $class = 'Priestess'; break;
                            }

                            $_SESSION["charDir"]=$charDat;
                            $_SESSION["charNum"]=$func->numDir($CHAR);
                            $_SESSION["charID"]=$charID;
                            $_SESSION["charName"]=$charName;
                            $_SESSION["charLevel"]=ord($charLevel);
                            $_SESSION["charClass"]=$class;


                    if ($_POST["vaibank"] == "1") {
                      header("location: index.php?sess=bank");
                      } else {
                    header("location: index.php");
                    }


                        }
                        else
                        {
                            $expName=explode("\x00",$charName);

                            $fRead=false;
                            $fOpen = fopen($charDat, "r");
                            while (!feof($fOpen)) {
                            @$fRead = "$fRead" . fread($fOpen, filesize($charDat) );
                            }
                            fclose($fOpen);

                            // Fill in 00 to left character
                            $addOnLeft=false;
                            $leftLen=32-strlen($expName[0]);
                            for($i=0;$i<$leftLen;$i++)
                            {
                                $addOnLeft.=pack("h*",00);
                            }
                            $writeName=$expName[0].$addOnLeft;

                            $sourceStr = substr($fRead, 0, 16) . $writeName . substr($fRead, 48);
                            $fOpen = fopen($charDat, "wb");
                            fwrite($fOpen, $sourceStr, strlen($sourceStr));
                            fclose($fOpen);


                            echo "CLEAR UP YOUR FILE, RE-ENTER!";
                        }


                    }
                    else
                    {
                        echo "ERRO ESTE CHAR NÃO É SEU, VOCÊ ESTA SENDO REDICIONADO.!";header("Location: index.php?sess=logout");
                    }


                }
                else
                {
                    echo "ERRO!! DESCULPE MAIS ESTE PERSONAGEM NÃO EXISTE OU (ARQUIVO CORROMPIDO) CHARDAT =>" . $charDat;
                }

                echo "<br><a href=\"$_SERVER[PHP_SELF]?$_SERVER[QUERY_STRING]\">Voltar</a>";

            }

            ?>
            </p>

                        <table width="97%" border="0" align="center" cellpadding="2" cellspacing="2">
                          <tr>
                             <td colspan="2" bgcolor="#ffcc00" height="25"><div align="center"><strong><font color="#000000">Personagens </font></strong>
                            </td>
                            </tr>
            <?
            if($_SESSION["charDir"])
            {
            $nickchar = $_SESSION["charName"];
            $query_clan = "SELECT * FROM [ClanDb].[dbo].[UL]  WHERE ChName = '$nickchar'";
            $q_clan = odbc_exec($connection, $query_clan);
            $clan = odbc_fetch_array($q_clan);
            $nomeclan = $clan['ClanName'];
            $numeroclan = $clan['MIconCnt'];

            ?>
                          <form method="post" onSubmit="disabledBttn(this)" action="index.php?sess=mudarnick">

                            <tr>
                                
                              <td align="right"><font color="#000000"><strong>NICK:</strong></font></td>
                              <td style="border-bottom: solid 1px #cecece;"><input class="col-md-3 form-control pull-left top-margin" type="text" size="15" maxlength="15" name="nick" value="<?=$_SESSION["charName"]?>"> <input type="hidden" name="acao" value="nick"><input class="btn btn-action btn-char top-margin" type="submit" class="padding_table" onclick ="return confirm('Tem certeza que deseja alterar a seu nick e pagar 20 Cr&eacute;ditos?')" value="Alterar">
                                <font color="#009900"><b>Valor: </b></font>20 Créditos <?=$func->img($_SESSION["ID"],"20")?></td>
                              </tr>
                            </form>

                    <tr>
                              <td align="right"><font color="#000000"><strong>CLASSE:</strong></font></td>
                              <form method="post" onSubmit="disabledBttn(this)" action="index.php?sess=mudarclasse">
                                <td style="border-bottom: solid 1px #cecece;"><img src="imgs/<?=$_SESSION["charClass"]?>.png" border="1" align="left">
                                  <select name="class" class="col-md-3 form-control btn-char pull-left top-margin">
                                    <option <?=($_SESSION["charClass"]=="Fighter")?"selected":""?>>Fighter</option>
                                    <option <?=($_SESSION["charClass"]=="Mechanician")?"selected":""?>>Mechanician</option>
                                    <option <?=($_SESSION["charClass"]=="Archer")?"selected":""?>>Archer</option>
                                    <option <?=($_SESSION["charClass"]=="Pikeman")?"selected":""?>>Pikeman</option>
                                    <option <?=($_SESSION["charClass"]=="Atalanta")?"selected":""?>>Atalanta</option>
                                    <option <?=($_SESSION["charClass"]=="Knight")?"selected":""?>>Knight</option>
                                    <option <?=($_SESSION["charClass"]=="Magician")?"selected":""?>>Magician</option>
                                    <option <?=($_SESSION["charClass"]=="Priestess")?"selected":""?>>Priestess</option>
                                    </select>
                                  <label>
                                    <input class="btn btn-action btn-char top-margin" name="acao" type="submit" class="padding_table" id="acao" onclick ="return confirm('Tem certeza que deseja alterar a sua classe e pagar 40 Cr&eacute;ditos?')" value="Alterar">
                                    </label>
                                  <input name="acao" type="hidden" id="acao" value="Alterar">
                                  <font color="#009900"><strong>Valor</strong></font>: 40 Cr&eacute;ditos
                                  <?=$func->img($_SESSION["ID"],"40")?></td>
                                </form>
                              </tr>
                      <tr>
                            <td align="right"><font color="#000000"><strong>LEVEL:</strong></font></td>
                            <td style="border-bottom: solid 1px #cecece;"><?=$_SESSION["charLevel"]?></td>
                            </tr>
                          <tr>
                         <? if (!$nomeclan) {} else { ?>
                          <form method="post" onSubmit="disabledBttn(this)" action="index.php?sess=sairclan">
                            <tr>
                             <td align="right"><font color="#000000"><strong>CLAN:</strong></font></td>
                             <td style="border-bottom: solid 1px #cecece;"><img src="/ClanContent/<? echo $numeroclan; ?>.bmp" width="20" height="20" border="1" align="absmiddle"> <strong><font color="Red"><? echo $nomeclan; ?></strong></font></td>
                            </tr>
                               </form>
                          <? } ?>
                          </tr>
                          <tr>            
                          <?
            }
            ?>
                          <tr>
                            <td align="right" width="15%"><strong><font color="#000000">PERSONAGENS:</font></strong></td>
                            <td style="border-bottom: solid 1px #cecece;"><span><?
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
                                    echo "<a href=\"?char=".$expValue[0]."\">".$expValue[0]."</a>&nbsp;&nbsp;";
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