<? if (Painel!=1) exit;

include_once "injection.php";

//Verificando se ele È lider de Clan
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

    if($_POST['acao']=="")
    {
?>
<link href="css/style.css" rel="stylesheet" type="text/css">
<table width="423" border="0" align="center" cellpadding="0" cellspacing="0">
  <tr>
    <td>
      <div align="center"><img src="imgs/clan_ger.png" /></div>
      <table width="423" border="0" align="center" cellpadding="6" cellspacing="0">
      <tr>
        <td>
		<table width="423" border="0" cellpadding="6" cellspacing="0" bgcolor="">
          <tr>
            <td width="40" align="center" valign="top"><font size="2" face="Verdana, Arial, Helvetica, sans-serif"><b><img src="http://<? echo $ipdoservidor; ?>/clancontent/<? echo $numeroclan; ?>.bmp" width="32" height="32" /></b></font></td>
            <td><font size="2" color="black" face="Verdana, Arial, Helvetica, sans-serif"><b>Clan: </b><? echo $nomeclan; ?><br />
              <b>LÌder: </b><? echo $nomelider; ?><br />
              <b>Criado dia:</b> <? echo $dataclan; ?><br />
              <b>Membros: </b><? echo $numeroMembros; ?></font></td>
          </tr>
        </table>
      <br />
      <table width="100%" border="0" cellspacing="0" cellpadding="2">
      <tr>
        <td bgcolor="#333333"><img src="imgs/setinha.gif" width="11" height="11" /><font size="2" face="Verdana, Arial, Helvetica, sans-serif" color="#fff"><b> Frase do Clan</font></b></td>
      </tr>
      <tr>
        <td><form name="form1" method="post" action="index.php?sess=admclan">
            <label>
            <input name="frase" type="text" id="frase" value="<? echo $fraseclan; ?>" size="50" maxlength="20">
            </label>
            <label>
            <input name="acao" type="submit" id="acao" value="Mudar">
            </label>
        </form></td>
      </tr>
    </table>
    <br>
    <table width="100%" border="0" cellspacing="0" cellpadding="2">
      <tr>
        <td bgcolor="#333333"><img src="imgs/setinha.gif" width="11" height="11" /><font size="2" face="Verdana, Arial, Helvetica, sans-serif" color="#fff"><b> Delegar a lÌderanÁa do Clan </font></b></td>
      </tr>
      <tr>
        <td><form action="index.php?sess=admclan" method="post" name="form1" id="form1">
            <font size="1" face="Verdana, Arial, Helvetica, sans-serif" color="#000000">Novo lÌder:</font>
            <select name="novolider" id="select2">
              <?
$query_new = "SELECT * FROM [ClanDb].[dbo].[UL]  WHERE ClanName = '$nomeclan' and ChName <> '$nomelider' AND [Permi] <> '2' order by JoinDate ASC";
$q_new = odbc_exec($connection, $query_new);
$qt_new = odbc_do($connection, $query_new);

while(odbc_fetch_row($qt_new)){
$char_new=odbc_result($qt_new,4);
echo "<option value='$char_new'>$char_new</option>";
}
?>
                        </select>
            <input name="acao" type="submit" id="acao" value="Delegar" />
            </label>
        </form></td>
      </tr>
    </table>
    <br />
    <table width="100%" border="0" cellspacing="0" cellpadding="2">
      <tr>
       <td bgcolor="#333333"><img src="imgs/setinha.gif" width="11" height="11" /><font size="2" face="Verdana, Arial, Helvetica, sans-serif" color="#fff"><b> Debandar Clan</font></b></td>
      </tr>
      <tr>
        <td><font size="1" face="Verdana, Arial, Helvetica, sans-serif" color="#000000">Esta a&ccedil;&atilde;o ir&aacute; fazer com que o seu clan seja excluido, e o nome do clan n&atilde;o poder&aacute; ser recriado por 15 dias de seu desban.</font></td>
      </tr>
      <tr>
        <td align="center"><form action="index.php?sess=admclan" method="post" name="form1" id="form2"><input name="acao" type="submit" id="acao" value="Debandar" onclick ="return confirm('Tem certeza de Debandar o Clan? Esta aÁ„o n„o poder· ser desfeita!')">
        </form></td>
      </tr>
    </table>
<?
}}
    //Indicando Vice Lider
    $vice_lider = anti_sqli($_POST['vice']);
    if($_POST['acao']=="Indicar")
	{
    //Indicando Vice-lider
    $sql_vice = "UPDATE [ClanDb].[dbo].[UL] SET [Permi]='2' WHERE [ChName]='$vice_lider'";
    $go_vice = odbc_exec($connection, $sql_vice);

    //Verificando se tinha antigo vice lider
    $vice_lider_anterior = anti_sqli($_POST['vice_anterior']);
    if (!$vice_lider_anterior) {} else {

    $sql_vice_ant = "UPDATE [ClanDb].[dbo].[UL] SET [Permi]='0' WHERE [ChName]='$vice_lider_anterior'";
    $go_vice_ant = odbc_exec($connection, $sql_vice_ant);
    }
 echo "<p><br><p><center><b>Vice-lider indicado:</b> $vice_lider <p><br><p>";
 echo "<meta HTTP-EQUIV='Refresh' CONTENT='3;URL=index.php?sess=admclan'>";


	}
    if ($_POST['acao']=="Mudar")
	{
$novafrase = $_POST[frase];

if (anti_sqli($novafrase) != 0) {
echo "<meta HTTP-EQUIV='Refresh' CONTENT='2;URL=index.php?sess=negado'>";
} else {

$novafrase = ereg_replace("[·‡‚„™]","a",$novafrase);
$novafrase = ereg_replace("[¡¿¬√]","A",$novafrase);
$novafrase = ereg_replace("[ÈËÍ]","e",$novafrase);
$novafrase = ereg_replace("[…» ]","E",$novafrase);
$novafrase = ereg_replace("[ÛÚÙı∫]","o",$novafrase);
$novafrase = ereg_replace("[”“‘’]","O",$novafrase);
$novafrase = ereg_replace("[˙˘˚]","u",$novafrase);
$novafrase = ereg_replace("[⁄Ÿ€]","U",$novafrase);
$novafrase = str_replace("Á","c",$novafrase);
$novafrase = str_replace("«","C",$novafrase);
$novafrase = str_replace("-","",$novafrase);
$novafrase = str_replace("«","C",$novafrase);

    function filtra_frase($ojb)
    {
        $badchars='.›`~!@$%^()+-_=[]{}\'\\:"|,/<>?'; # Nhung ky tu khong cho phep duoc len danh sach tai day
        for ($i=0;$i<strlen($badchars);$i++){
            $ojb=str_replace($badchars[$i],"",$ojb);

        }

        return $ojb;
    }
    $novafrase = filtra_frase("$novafrase");


				//Mudando a Frase do Clan
				$query = "UPDATE [ClanDb].[dbo].[CL] SET [Note]='$novafrase' WHERE [UserID]='$nomedochar'";
                                $q = odbc_exec($connection, $query);
                                if($q)
                                {
								echo "<table width=100% border=0 cellspacing=0 cellpadding=10><tr><td align=center bgcolor=#999999><b>Frase do Clan Atualizada!</b></td>		</tr></table>
";
								echo "<meta HTTP-EQUIV='Refresh' CONTENT='2;URL=index.php?sess=admclan'>";
								}


		}
    }

        }

//Debandar clan
    if($_POST['acao']=="Debandar")
	{
     //Deleta clan clan
$sq_delclan = "DELETE [ClanDb].[dbo].[CL] WHERE [ClanName]='$nomeclan' ";
$go_delclan = odbc_exec($connection, $sq_delclan);

//Deleta membros do clan
$sql_delclanmember = "DELETE [ClanDb].[dbo].[UL] WHERE [ClanName]='$nomeclan' ";
$go_delclanmember = odbc_exec($connection, $sql_delclanmember);

        echo "<p><br><p><b><center>O clan $nomeclan foi Debandado!<p><br><p>";
        echo "<meta HTTP-EQUIV='Refresh' CONTENT='3;URL=index.php'>";

        //Deleta a Tag clan
        $caminho ="C:/Inetpub/wwwroot/ClanContent";
        if(file_exists("$caminho/$numeroclan")) { unlink($caminho.'/'.$numeroclan); }

$ip = $_SERVER["REMOTE_ADDR"];

//Gravando Logs de Deban
$sql_logreg = "INSERT INTO [ArcadePT].[dbo].[ClanDebandar] ([clan_nome],[clan_liderID],[clan_liderNick],[clan_numero],[clan_debandou],[IP])
values('$nomeclan','$idlider','$nomelider','$numeroclan',getdate(),'$ip')";
$go_logreg = odbc_exec($connection, $sql_logreg);



    }

//Trocando o lider do clan

    if($_POST['acao']=="Delegar")
	{
	//verificando se o indicado faz parte do clan
    $vice_lider = anti_sqli($_POST['novolider']);
    //Verifica se o lider indicado faz parte do clan
    $query_vm = "SELECT * FROM [ClanDb].[dbo].[UL] WHERE [ChName]='$vice_lider'";
    $q_vm = odbc_exec($connection, $query_vm);
    $i_vm=0;
    while($linha = odbc_fetch_array($q_vm)) {
    $i_vm++;
    $vice_lider_ID = $linha['userid'];
    }

    if ($i_vm) {
    //Atualiza a tabela com o novo lider
    $sql_novolider = "UPDATE [ClanDb].[dbo].[CL] SET [UserID]='$vice_lider_ID',[ClanZang]='$vice_lider' WHERE [MIconCnt]='$numeroclan'";
    $go_vice_novolider = odbc_exec($connection, $sql_novolider);
    echo "<font color='white'><b><p><br><p><center>Seu char $nomelider n„o È mais lider do clan $nomeclan, o cargo agora È ocupado pelo player $vice_lider.<p><br><p></font>";
    echo "<meta HTTP-EQUIV='Refresh' CONTENT='4;URL=index.php'>";

          } else {
    //Aqui ele vai dizer que o char indicado para ser novo lider n„o faz parte do clan
    echo "<font color='white'><b><p><br><p><center><font color='white'>O char $vice_lider n„o faz parte do clan $nomeclan!</font><p><br><p></font>";
          }

    }
?>
    </td>
  </tr>
</table>