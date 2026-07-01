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
            $tier0_1 = ord(substr($fRead,0x1fd,2));
            $tier0_2 = ord(substr($fRead,0x1fe,2));
            $tier0_3 = ord(substr($fRead,0x1ff,2));
            $tier0_4 = ord(substr($fRead,0x200,2));

            $tier1_1 = ord(substr($fRead,0x201,2));
            $tier1_2 = ord(substr($fRead,0x202,2));
            $tier1_3 = ord(substr($fRead,0x203,2));
            $tier1_4 = ord(substr($fRead,0x204,2));

            $tier2_1 = ord(substr($fRead,0x205,2));
            $tier2_2 = ord(substr($fRead,0x206,2));
            $tier2_3 = ord(substr($fRead,0x207,2));
            $tier2_4 = ord(substr($fRead,0x208,2));

            $tier3_1 = ord(substr($fRead,0x209,2));
            $tier3_2 = ord(substr($fRead,0x20A,2));
            $tier3_3 = ord(substr($fRead,0x20B,2));
            $tier3_4 = ord(substr($fRead,0x1fc,2));

            $defaultSP=$func->checkSkillPoints($_SESSION["charLevel"],'SP');
            $defaultEP=$func->checkSkillPoints($_SESSION["charLevel"],'EP');

            $totalSP=$tier0_1+$tier0_2+$tier0_3+$tier0_4+
            $tier1_1+$tier1_2+$tier1_3+$tier1_4+
            $tier2_1+$tier2_2+$tier2_3+$tier2_4;

            $totalEP=$tier3_1+$tier3_2+$tier3_3+$tier3_4;

            if ($_SESSION["charClass"]=="Fighter")
            {
            	$src="imgs/skill/fs/";

				$t0_1="<br>Meele <br> Mastery";
				$t0_2="<br>Fire <br> Attribute";
				$t0_3="<br>Raving<br>_ ";
				$t0_4="<br>Impact<br>_ ";

				$t1_1="<br>Tripple<br> Impact";
				$t1_2="<br>Brutal<br> Swing";
				$t1_3="<br>Roar<br>_";
				$t1_4="<br>Rage<br> Zecram";

				$t2_1="<br>Concentration<br>_";
				$t2_2="<br>Avenging<br> Crash";
				$t2_3="<br>Swift<br> Axe";
				$t2_4="<br>Bone<br> Crash";

				$t3_1="<br>Destroyer<br>_";
				$t3_2="<br>Berserker<br>_";
				$t3_3="<br>Cyclone<br> Strike";
				$t3_4="<br>Boost<br> Health";

            }
        	elseif ($_SESSION["charClass"]=="Mechanician")
            {

            	$src="imgs/skill/ms/";

				$t0_1="<br>Extreme<br> Shield";
				$t0_2="<br>Mechanic<br> Bomb";
				$t0_3="<br>Poison<br> Attribute";
				$t0_4="<br>Physical<br> Absorbtion";

				$t1_1="<br>Great<br> Smash";
				$t1_2="<br>Maximize<br>_";
				$t1_3="<br>Automation<br>_";
				$t1_4="<br>Spark<br>_";

				$t2_1="<br>Metal<br> Armor";
				$t2_2="<br>Grand<br> Smash";
				$t2_3="<br>Mechanic<br> Mastery";
				$t2_4="<br>Spark<br> Shield";

				$t3_1="<br>Impulsion<br>_";
				$t3_2="<br>Compulsion<br>_";
				$t3_3="<br>Magnetic<br> Sphere";
				$t3_4="<br>Metal<br> Golem";


            }
        	elseif ($_SESSION["charClass"]=="Archer")
            {

            	$src="imgs/skill/as/";

				$t0_1="<br>Scout<br> Hawk";
				$t0_2="<br>Shooting<br> Mastery";
				$t0_3="<br>Wind<br> Arrow";
				$t0_4="<br>Perfect <br> Aim";

				$t1_1="<br>Dion's<br> Eye";
				$t1_2="<br>Falcon<br>_";
				$t1_3="<br>Arrow<br> Of Rage";
				$t1_4="<br>Avalanche<br>_";

				$t2_1="<br>Elemental<br> Shot";
				$t2_2="<br>Golden<br> Falcon";
				$t2_3="<br>Bomb<br> Shot";
				$t2_4="<br>Perforation<br>_";

				$t3_1="<br>Wolverine<br>_";
				$t3_2="<br>Evasion<br> Master";
				$t3_3="<br>Phoenix<br> Shot";
				$t3_4="<br>Force Of<br> Nature";

            }
        	elseif ($_SESSION["charClass"]=="Pikeman")
            {

            	$src="imgs/skill/ps/";

				$t0_1="<br>Pike<br> Wind";
				$t0_2="<br>Ice<br> Attriute";
				$t0_3="<br>Critical<br> Hit";
				$t0_4="<br>Jumping<br> Crash";

				$t1_1="<br>Ground<br> Pike";
				$t1_2="<br>Tornado<br>_";
				$t1_3="<br>Block<br> Mastery";
				$t1_4="<br>Expansion<br>_";

				$t2_1="<br>Venom<br> Spear";
				$t2_2="<br>Vanish<br>_";
				$t2_3="<br>Critical<br> Mastery";
				$t2_4="<br>Chain<br> Lance";

				$t3_1="<br>Assasin's<br> Eye";
				$t3_2="<br>Charging<br> Strike";
				$t3_3="<br>Vague<br>_";
				$t3_4="<br>Shadow<br> Master";

            }
        	elseif ($_SESSION["charClass"]=="Atalanta")
            {

            	$src="imgs/skill/at/";

				$t0_1="<br>Shield<br> Strike";
				$t0_2="<br>Farina<br>_";
				$t0_3="<br>Throwing<br> Mastery";
				$t0_4="<br>Vigor<br> Spear";

				$t1_1="<br>Windy<br>_";
				$t1_2="<br>Twisted<br> Javelin";
				$t1_3="<br>Soul<br> Sucker";
				$t1_4="<br>Fire<br> Javelin";

				$t2_1="<br>Split<br> Javelin";
				$t2_2="<br>Trimph Of<br> Valhalla";
				$t2_3="<br>Light<br> Javelin";
				$t2_4="<br>Storm<br> Javelin";

				$t3_1="<br>Hall Of<br> Valhalla";
				$t3_2="<br>Extreme<br> Rage";
				$t3_3="<br>Frost<br> Javelin";
				$t3_4="<br>Vengeance<br>_";

            }
        	elseif ($_SESSION["charClass"]=="Knight")
            {

            	$src="imgs/skill/ks/";

				$t0_1="<br>Sword<br> Blast";
				$t0_2="<br>Holy<br> Body";
				$t0_3="<br>Physical<br> Training";
				$t0_4="<br>Double<br>Crash";

				$t1_1="<br>Holy<br>Valor";
				$t1_2="<br>Brandish<br>_";
				$t1_3="<br>Piercing<br>_";
				$t1_4="<br>Drastic<br> Spirit";

				$t2_1="<br>Sword<br> Mastery";
				$t2_2="<br>Devine<br> Shield";
				$t2_3="<br>Holy<br> Incantation";
				$t2_4="<br>Grand<br> Cross";

				$t3_1="<br>Sword Of<br> Justice";
				$t3_2="<br>Godly<br> Shield";
				$t3_3="<br>God's<br> Blessing";
				$t3_4="<br>Divine<br> Piercing";

            }
        	elseif ($_SESSION["charClass"]=="Magician")
            {

            	$src="imgs/skill/mgs/";

				$t0_1="<br>Agony<br>_";
				$t0_2="<br>Fire<br> Bolt";
				$t0_3="<br>Zenith<br>_";
				$t0_4="<br>Fire<br> Ball";

				$t1_1="<br>Mental<br> Mastery";
				$t1_2="<br>Waternado<br>_";
				$t1_3="<br>Enchant<br> Weapon";
				$t1_4="<br>Death<br> Ray";

				$t2_1="<br>Enery<br> Shield";
				$t2_2="<br>Diastrophism<br>_";
				$t2_3="<br>Spirit<br> Elemental";
				$t2_4="<br>Dancing<br> Sword";

				$t3_1="<br>Fire<br> Elemental";
				$t3_2="<br>Flame<br> Wave";
				$t3_3="<br>Distortion<br>_";
				$t3_4="<br>Meteorite<br>_";

            }
        	elseif ($_SESSION["charClass"]=="Priestess")
        	{

        		$src="imgs/skill/prs/";

				$t0_1="<br>Healing<br>_";
				$t0_2="<br>Holy<br> Bolt";
				$t0_3="<br>Multi<br> Spark";
				$t0_4="<br>Holy<br> Mind";

				$t1_1="<br>Meditation<br>_";
				$t1_2="<br>Divine<br> Lightening";
				$t1_3="<br>Holy<br> Reflection";
				$t1_4="<br>Grand<br> Healing";

				$t2_1="<br>Vigor<br> Ball";
				$t2_2="<br>Resurrection<br>_";
				$t2_3="<br>Extinction<br>_";
				$t2_4="<br>Virtual<br> Life";

				$t3_1="<br>Glacial<br> Spike";
				$t3_2="<br>Regen<br> Field";
				$t3_3="<br>Chain<br> Lightening";
				$t3_4="<br>Summon<br> Muspell";

        	}
        	else
        	{$src="imgs/skill/";}

            if(anti_sqli($_POST[action]!="skill"))
            {
?>
            </div>

            <hr>
            <form method="post" onSubmit="disabledBttn(this)" action="<?=$_SERVER[PHP_SELF]."?".$_SERVER[QUERY_STRING]?>">
            <table width="100%" border="0" align="center" cellpadding="2" cellspacing="2">
              <tr>
                <td><b><img src="imgs/setinha.gif" width="11" height="11"><strong> GERENCIAMENTO DOS PONTOS DE SKILLS</strong></b></td>
              </tr>

              <tr>
                <td><table width="100%" border="0" cellspacing="0" cellpadding="4" class="padding_all">
                  <tr>
                    <td colspan="4" style="font-weight: bolder;" bgcolor="#fbd97f">1&ordf; Tier</td>
                  </tr>
                  <tr>
                    <td bgcolor="#e5e5e5"><img src="imgs/skill/<?=$_SESSION["charClass"]?>_01.jpg" width="39" height="39" align="middle">
                      <select name="tier0_1">
                      <?
                for($i=1; $i<=10; $i++)
                {
                    echo "<option ". ( ($tier0_1==$i)?"selected":"" ) .">". $i ."</option>";
                }
?>
                    </select>
                    <?echo "<b>".$t0_1."</b>"?></td>
                    <td bgcolor="#e5e5e5"><img src="imgs/skill/<?=$_SESSION["charClass"]?>_02.jpg" width="39" height="39" align="middle">
                      <select name="tier0_2">
                      <?
                for($i=1; $i<=10; $i++)
                {
                    echo "<option ". ( ($tier0_2==$i)?"selected":"" ) .">". $i ."</option>";
                }
?>
                    </select>
                    <?echo "<b>".$t0_2."</b>"?></td>
                    <td bgcolor="#e5e5e5"><img src="imgs/skill/<?=$_SESSION["charClass"]?>_03.jpg" width="39" height="39" align="middle">
                      <select name="tier0_3">
                      <?
                for($i=1; $i<=10; $i++)
                {
                    echo "<option ". ( ($tier0_3==$i)?"selected":"" ) .">". $i ."</option>";
                }
?>
                    </select>
                    <?echo "<b>".$t0_3."</b>"?></td>
                    <td bgcolor="#e5e5e5"><img src="imgs/skill/<?=$_SESSION["charClass"]?>_04.jpg" width="39" height="39" align="absmiddle">
                      <select name="tier0_4">
                      <?
                for($i=1; $i<=10; $i++)
                {
                    echo "<option ". ( ($tier0_4==$i)?"selected":"" ) .">". $i ."</option>";
                }
?>
                    </select>
                    <?echo "<b>".$t0_4."</b>"?></td>
                  </tr>
                  <tr>
                    <td colspan="4">&nbsp;</td>
                  </tr>
                  <tr>
                    <td colspan="4" style="font-weight: bolder;" bgcolor="#fccc49">2&ordf; Tier</td>
                  </tr>
                  <tr>
                    <td bgcolor="#e5e5e5"><img src="imgs/skill/<?=$_SESSION["charClass"]?>_05.jpg" width="39" height="39" align="absmiddle">
                      <select name="tier1_1">
                      <?
                for($i=1; $i<=10; $i++)
                {
                    echo "<option ". ( ($tier1_1==$i)?"selected":"" ) .">". $i ."</option>";
                }
?>
                    </select>
                    <?echo "<b>".$t1_1."</b>"?></td>
                    <td bgcolor="#e5e5e5"><img src="imgs/skill/<?=$_SESSION["charClass"]?>_06.jpg" width="39" height="39" align="absmiddle">
                      <select name="tier1_2">
                      <?
                for($i=1; $i<=10; $i++)
                {
                    echo "<option ". ( ($tier1_2==$i)?"selected":"" ) .">". $i ."</option>";
                }
?>
                    </select>
                    <?echo "<b>".$t1_2."</b>"?></td>
                    <td bgcolor="#e5e5e5"><img src="imgs/skill/<?=$_SESSION["charClass"]?>_07.jpg" width="39" height="39" align="absmiddle">
                      <select name="tier1_3">
                      <?
                for($i=1; $i<=10; $i++)
                {
                    echo "<option ". ( ($tier1_3==$i)?"selected":"" ) .">". $i ."</option>";
                }
?>
                    </select>
                    <?echo "<b>".$t1_3."</b>"?></td>
                    <td bgcolor="#e5e5e5"><img src="imgs/skill/<?=$_SESSION["charClass"]?>_08.jpg" width="39" height="39" align="absmiddle">
                      <select name="tier1_4">
                      <?
                for($i=1; $i<=10; $i++)
                {
                    echo "<option ". ( ($tier1_4==$i)?"selected":"" ) .">". $i ."</option>";
                }
?>
                    </select>
                    <?echo "<b>".$t1_4."</b>"?></td>
                  </tr>
                  <tr>
                    <td colspan="4">&nbsp;</td>
                  </tr>
                  <tr bgcolor="#ffba00">
                    <td colspan="4" style="font-weight: bolder;">3&ordf; Tier</td>
                  </tr>
                  <tr>
                    <td bgcolor="#e5e5e5"><img src="imgs/skill/<?=$_SESSION["charClass"]?>_09.jpg" width="39" height="39" align="absmiddle">
                      <select name="tier2_1">
                      <?
                for($i=1; $i<=10; $i++)
                {
                    echo "<option ". ( ($tier2_1==$i)?"selected":"" ) .">". $i ."</option>";
                }
?>
                    </select>
                    <?echo "<b>".$t2_1."</b>"?></td>
                    <td bgcolor="#e5e5e5"><img src="imgs/skill/<?=$_SESSION["charClass"]?>_10.jpg" width="39" height="39" align="absmiddle">
                      <select name="tier2_2">
                      <?
                for($i=1; $i<=10; $i++)
                {
                    echo "<option ". ( ($tier2_2==$i)?"selected":"" ) .">". $i ."</option>";
                }
?>
                    </select>
                    <?echo "<b>".$t2_2."</b>"?></td>
                    <td bgcolor="#e5e5e5"><img src="imgs/skill/<?=$_SESSION["charClass"]?>_11.jpg" width="39" height="39" align="absmiddle">
                      <select name="tier2_3">
                      <?
                for($i=1; $i<=10; $i++)
                {
                    echo "<option ". ( ($tier2_3==$i)?"selected":"" ) .">". $i ."</option>";
                }
?>
                    </select>
                    <?echo "<b>".$t2_3."</b>"?></td>
                    <td bgcolor="#e5e5e5"><img src="imgs/skill/<?=$_SESSION["charClass"]?>_12.jpg" width="39" height="39" align="absmiddle">
                      <select name="tier2_4">
                      <?
                for($i=1; $i<=10; $i++)
                {
                    echo "<option ". ( ($tier2_4==$i)?"selected":"" ) .">". $i ."</option>";
                }
?>
                    </select>
                    <?echo "<b>".$t2_4."</b>"?></td>
                  </tr>
                  <tr>
                    <td colspan="4">&nbsp;</td>
                  </tr>
                  <tr bgcolor="#ffa200">
                    <td colspan="4" style="font-weight: bolder;">4&ordf; Tier</td>
                  </tr>
                  <tr>
                    <td bgcolor="#e5e5e5"><img src="imgs/skill/<?=$_SESSION["charClass"]?>_13.jpg" width="39" height="40" align="absmiddle">
                      <select name="tier3_1">
                      <?
                for($i=1; $i<=10; $i++)
                {
                    echo "<option ". ( ($tier3_1==$i)?"selected":"" ) .">". $i ."</option>";
                }
?>
                    </select>
                    <?echo "<b>".$t3_1."</b>"?></td>
                    <td bgcolor="#e5e5e5"><img src="imgs/skill/<?=$_SESSION["charClass"]?>_14.jpg" width="39" height="40" align="absmiddle">
                      <select name="tier3_2">
                      <?
                for($i=1; $i<=10; $i++)
                {
                    echo "<option ". ( ($tier3_2==$i)?"selected":"" ) .">". $i ."</option>";
                }
?>
                    </select>
                    <?echo "<b>".$t3_2."</b>"?></td>
                    <td bgcolor="#e5e5e5"><img src="imgs/skill/<?=$_SESSION["charClass"]?>_15.jpg" width="39" height="40" align="absmiddle">
                      <select name="tier3_3">
                      <?
                for($i=1; $i<=10; $i++)
                {
                    echo "<option ". ( ($tier3_3==$i)?"selected":"" ) .">". $i ."</option>";
                }
?>
                    </select>
                    <?echo "<b>".$t3_3."</b>"?></td>
                    <td bgcolor="#e5e5e5"><img src="imgs/skill/<?=$_SESSION["charClass"]?>_16.jpg" width="39" height="40" align="absmiddle">
                      <select name="tier3_4">
                      <?
                for($i=1; $i<=10; $i++)
                {
                    echo "<option ". ( ($tier3_4==$i)?"selected":"" ) .">". $i ."</option>";
                }
?>
                    </select>
                    <?echo "<b>".$t3_4."</b>"?></td>
                  </tr>
                </table></td>
              </tr>
              <tr>
                <td align="left"><div align="center"><strong>Pontos Simples:
                <?=$defaultSP-$totalSP?>
                / Pontos Especiais:
                <?=$defaultEP-$totalEP?>
                </strong></div></td>
              </tr>

              <tr>
                <td align="center"><input type="submit" value="Distribuir e Treinar Skills" class="button"></td>
              </tr>
            </table>
            <input type="hidden" name="action" value="skill">
            <hr>
            </form>

<?
            }
            else
            {
                $checkSubmitSP=anti_sqli($_POST['tier0_1'])+anti_sqli($_POST['tier0_2'])+anti_sqli($_POST['tier0_3'])+anti_sqli($_POST['tier0_4'])+
            anti_sqli($_POST['tier1_1'])+anti_sqli($_POST['tier1_2'])+anti_sqli($_POST['tier1_3'])+anti_sqli($_POST['tier1_4'])+
            anti_sqli($_POST['tier2_1'])+anti_sqli($_POST['tier2_2'])+anti_sqli($_POST['tier2_3'])+anti_sqli($_POST['tier2_4']);

                $checkSubmitEP=anti_sqli($_POST['tier3_1'])+anti_sqli($_POST['tier3_2'])+anti_sqli($_POST['tier3_3'])+anti_sqli($_POST['tier3_4']);

                if( ($checkSubmitSP>$defaultSP) || ($checkSubmitEP>$defaultEP))
                {
                    echo "<b><div align=\"center\">VOCÊ ENVIOU UMA SP (". $checkSubmitSP .") QUE É MAIOR QUE A SP PERMITIDA COM (". $defaultSP .") ! </div>";
                    echo "<b><div align=\"center\">VOCÊ ENVIOU UMA EP (". $checkSubmitEP .") QUE É MAIOR QUE A EP PERMITIDA COM (". $defaultEP .") !</div><br><br>";
                }
                else
                {
                    $tier0_1=trim(pack("i",$_POST['tier0_1']),"\x00");
                    $tier0_2=trim(pack("i",$_POST['tier0_2']),"\x00");
                    $tier0_3=trim(pack("i",$_POST['tier0_3']),"\x00");
                    $tier0_4=trim(pack("i",$_POST['tier0_4']),"\x00");

                    $tier1_1=trim(pack("i",$_POST['tier1_1']),"\x00");
                    $tier1_2=trim(pack("i",$_POST['tier1_2']),"\x00");
                    $tier1_3=trim(pack("i",$_POST['tier1_3']),"\x00");
                    $tier1_4=trim(pack("i",$_POST['tier1_4']),"\x00");

                    $tier2_1=trim(pack("i",$_POST['tier2_1']),"\x00");
                    $tier2_2=trim(pack("i",$_POST['tier2_2']),"\x00");
                    $tier2_3=trim(pack("i",$_POST['tier2_3']),"\x00");
                    $tier2_4=trim(pack("i",$_POST['tier2_4']),"\x00");

                    $tier3_1=trim(pack("i",$_POST['tier3_1']),"\x00");
                    $tier3_2=trim(pack("i",$_POST['tier3_2']),"\x00");
                    $tier3_3=trim(pack("i",$_POST['tier3_3']),"\x00");
                    $tier3_4=trim(pack("i",$_POST['tier3_4']),"\x00");

                    $skillStr=$tier3_4.$tier0_1.$tier0_2.$tier0_3.$tier0_4.
                        $tier1_1.$tier1_2.$tier1_3.$tier1_4.
                        $tier2_1.$tier2_2.$tier2_3.$tier2_4.
                        $tier3_1.$tier3_2.$tier3_3;

                    $fRead=false;
                    $fOpen = fopen($_SESSION["charDir"], "r");
                    while (!feof($fOpen)) {
                    @$fRead = "$fRead" . fread($fOpen, filesize($_SESSION["charDir"]) );
                    }
                    fclose($fOpen);

                    $sourceStr = substr($fRead, 0, 508) . $skillStr . substr($fRead, 524, 0) . ($func->charFullMastery()) . substr($fRead, 556);
                    $fOpen = fopen($_SESSION["charDir"], "wb");
                    fwrite($fOpen, $sourceStr, strlen($sourceStr));
                    fclose($fOpen);

                    echo "<b><div align=\"center\"><font color='Green'>Todas as skills foram distribuidas conforme você distribuiu e forão treinadas!</font></div><br><br>";

                }

                echo "<b><div align=\"center\"><a href=\"$_SERVER[PHP_SELF]?$_SERVER[QUERY_STRING]\">VOLTAR</a></div><br><br>";
            }

        }

?>


