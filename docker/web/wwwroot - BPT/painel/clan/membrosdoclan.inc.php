<? if (Painel!=1) exit;

include_once "injection.php";
include_once "incluir/configura.php";

//Verificando se ele  lider de Clan

$nomedochar = $_SESSION["ID"];
$connection = odbc_connect( $connection_string, $user, $pass );
$query = "SELECT * FROM [ClanDb].[dbo].[CL]  WHERE UserID = '$nomedochar' ";
$qLider = odbc_exec($connection, $query);
$qtLider = odbc_do($connection, $query);

$i = 0;
while(odbc_fetch_row($qtLider)){
//ClanName
$nomeclan=odbc_result($qtLider,2);
//Note
$fraseclan=odbc_result($qtLider,3);
//UserID
$idlider=odbc_result($qtLider,5);
//ClanZang
$nomelider=odbc_result($qtLider,6);
//MemCnt
$numeroMembros=odbc_result($qtLider,8);
//MIconCnt
$numeroclan=odbc_result($qtLider,9);
//RegiDate
$dataclan=odbc_result($qtLider,10);

$dataclan = substr($dataclan, 0, 10);
$explode = explode("-", $dataclan);
$dataclan = "$explode[2]/$explode[1]/$explode[0]";


$i++;
if($i==0) {

echo "<meta HTTP-EQUIV='Refresh' CONTENT='0;URL=index.php'>";

} else {

//Kickando os Players do Clan
if($_POST['acao']=="kickar") {

$connection = odbc_connect( $connection_string, $user, $pass );
$var = $_POST["nomes"];
for($i = 0; $i < count($var); $i++){
$sql = ("DELETE FROM [ClanDB].[dbo].[UL] WHERE ChName = '$var[$i]'");
//Diminui membros da tabela do clan
$numeroMembros = $numeroMembros-1;
$sql2 = ("UPDATE [ClanDB].[dbo].[CL] SET [MemCnt] = '$numeroMembros'  WHERE ClanName = '$nomeclan'");
$kick = odbc_exec($connection, $sql);
$atualizar = odbc_exec($connection, $sql2);
echo "<center><br>Player $var[$i] Kickado do Clan <br></center>";
echo "<meta HTTP-EQUIV='Refresh' CONTENT='3;URL=index.php?sess=membros'>";
}
$numero_de_players_kikados = count($var);
echo "Total: ".$numero_de_players_kikados." players kikados do servidor.";
}else{

?>
<link href="css/style.css" rel="stylesheet" type="text/css">
<table width="423" border="0" align="center" cellpadding="0" cellspacing="0">
  <tr>
    <td>
      <div align="center"><img src="imgs/clan_ger.png" /></div>
      <table width="423" border="0" align="center" cellpadding="6" cellspacing="0">
      <tr>
        <td><form action="index.php?sess=membros&amp;" method="post" name="form1" id="form1">
          <table width="423" border="0" cellpadding="6" cellspacing="0" bgcolor="">
            <tr>
              <td width="40" align="center" valign="top" bgcolor=""><font size="2" face="Verdana, Arial, Helvetica, sans-serif"><b><img src="/ClanContent/<? echo $numeroclan; ?>.bmp" width="32" height="32" /></b></font></td>
              <td bgcolor=""><font size="2" color="black" face="Verdana, Arial, Helvetica, sans-serif"><b>Clan: </b><? echo $nomeclan; ?><br />
                <b>Lder: </b><? echo $nomelider; ?><br />
                <b>Membros: </b><? echo $numeroMembros; ?></font></td>
            </tr>
          </table>
          <table width="423" border="0" cellpadding="3" cellspacing="0" bordercolor="#999999">
            <tr>
              <td height="30" colspan="7" align="center" bgcolor="#333333"><font size="2" face="Verdana, Arial, Helvetica, sans-serif"><b><font color="#FFFFFF">Lista de Membros do Clan</font></b></font></td>
              </tr>
            <tr>
              <td width="46%" bgcolor="#333333"><font size="2" face="Verdana, Arial, Helvetica, sans-serif"><b><span class="branco">Nick</span></b></font></td>
              <td width="17%" align="center" bgcolor="#333333"><font size="2" face="Verdana, Arial, Helvetica, sans-serif"><b><span class="branco">Classe</span></b></font></td>
              <td width="12%" align="center" bgcolor="#333333"><font size="2" face="Verdana, Arial, Helvetica, sans-serif"><b><span class="branco">Level</span></b></font></td>
              <td width="25%" align="center" bgcolor="#333333"><font size="2" face="Verdana, Arial, Helvetica, sans-serif"><b><span class="branco">Data inclus&atilde;o</span></b></font></td>
              <td width="12%" align="center" bgcolor="#333333"><font size="2" face="Verdana, Arial, Helvetica, sans-serif"><b><span class="branco">Status</span></b></font></td>
              <td width="13%" align="center" bgcolor="#333333"><font size="2" face="Verdana, Arial, Helvetica, sans-serif"><b><span class="branco">Kickar</span></b></font></td>
              </tr>
            <?
$connection = odbc_connect( $connection_string, $user, $pass );
$query_member = "SELECT * FROM [ClanDb].[dbo].[UL]  WHERE ClanName = '$nomeclan' and ChName <> '$nomelider' order by JoinDate ASC";
$qmember = odbc_exec($connection, $query_member);
$qtmember = odbc_do($connection, $query_member);


while(odbc_fetch_row($qtmember)){
$charmember=odbc_result($qtmember,4);
$classemember=odbc_result($qtmember,5);
$levelmember=odbc_result($qtmember,6);
$invitemember=odbc_result($qtmember,9);


$invitemember = substr($invitemember, 0, 10);
$conv = explode("-", $invitemember);
$invitemember = "$conv[2]/$conv[1]/$conv[0]";

//Checar se eles estao online ou offline
$query_online = "SELECT * FROM [ClanDb].[dbo].[CT] WHERE ChName = '$charmember'";
$membon = odbc_do($connection, $query_online);
$estaon = 0;
while(odbc_fetch_row($membon)) $estaon++;

?>
            <tr>
              <td bgcolor="#999999"><? echo $charmember; ?></td>
              <td align="center" bgcolor="#999999"><img src="imgs/0<? echo $classemember; ?>.gif" width="29" height="26" border="0" /></font></td>
              <td align="center" bgcolor="#999999"><? echo $levelmember; ?></td>
              <td align="center" bgcolor="#999999"><? echo $invitemember; ?></td>
              <td align="center" bgcolor="#999999"><? if($estaon == 0) { echo "<img src='imgs/off_member.gif' >";} else { echo "<img src='imgs/on_member.gif' >";} ?></td>
              <td align="center" bgcolor="#990000">Sim<input name="nomes[]" type="checkbox" id="nomes[]" value="<? echo $charmember; ?>" /></td>
              </tr>
            <? } ?>
          </table>
          <center>
            <font size="2" face="Verdana, Arial, Helvetica, sans-serif">
              <input type="hidden" name="acao" value="kickar" />
              <input type="submit" name="button" id="button" value="Alterar Clan"  class="button"/>
              </font>
          </center>
        </form>
          <p>&nbsp;</p></td>
      </tr>
    </table></td>
  </tr>
</table>
<?
}}} 
?>