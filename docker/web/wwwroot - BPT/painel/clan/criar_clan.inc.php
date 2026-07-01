<? if (Painel!=1) exit; ?>
<table width="423" border="0" align="center" cellpadding="0" cellspacing="0">
  <tr>
    <td height="30" align="center"><img src="imgs/top_cria_clan.gif" width="420" height="98" /></td>
  </tr>
  <tr>
    <td height="100" align="center"><b>


<?
include_once "injection.php";
//[ClanName]
$clan_nome = anti_sqli($_POST['nomeclan']);

if (!$_POST['nomeclan']) {

echo "<font color='#000000'>Você não informou um nome de clan, você será redirecionado para escrever o nome do clan!</font>";
echo "<meta HTTP-EQUIV='Refresh' CONTENT='3;URL=index.php?sess=criaclan'>";

} else {


if (anti_sql($clan_nome) != 0) {
echo "<meta HTTP-EQUIV='Refresh' CONTENT='2;URL=index.php?sess=criaclan'>";
echo "<font color='#000000'>O nome que tentou utilizar para clan FOI NEGADO! Tentou utilizar outro</font>";
} else {


$clan_nome = ereg_replace("[áàâãª]","a",$clan_nome);
$clan_nome = ereg_replace("[ÁÀÂÃ]","A",$clan_nome);
$clan_nome = ereg_replace("[éèê]","e",$clan_nome);
$clan_nome = ereg_replace("[ÉÈÊ]","E",$clan_nome);
$clan_nome = ereg_replace("[óòôõº]","o",$clan_nome);
$clan_nome = ereg_replace("[ÓÒÔÕ]","O",$clan_nome);
$clan_nome = ereg_replace("[úùû]","u",$clan_nome);
$clan_nome = ereg_replace("[ÚÙÛ]","U",$clan_nome);
$clan_nome = str_replace("ç","c",$clan_nome);
$clan_nome = str_replace("Ç","C",$clan_nome);
$clan_nome = str_replace("-","",$clan_nome);
$clan_nome = str_replace("Ç","C",$clan_nome);

    function filtra_frase($ojb)
    {
        $badchars='.Ý`~!@$%^()+-_=[]{}\'\\:"|,/<>?'; # Nhung ky tu khong cho phep duoc len danh sach tai day
        for ($i=0;$i<strlen($badchars);$i++){
            $ojb=str_replace($badchars[$i],"",$ojb);

        }

        return $ojb;
    }

$clan_nome = filtra_frase("$clan_nome");
$clan_nome = str_replace(" ","",$clan_nome);



$clan_liderID = $_SESSION["ID"];
$clan_liderNick = $_SESSION["charName"];

//Level do char que player deseja ter como lider
$level_lider = $_SESSION["charLevel"];

 //verificando se o player já tem algum char lider na sua conta
 if($lider_clan==0) {

 if($level_lider< 40) {
 //Aqui diz que o player indicado como lider não tem level suficiente para criar clans
echo "<font color='#000000'>O seu char $clan_liderNick é level $level_lider, para criar clans seu char que será líder deve possuir level 40+</font>";

 } else {
 //Pegando o saldo em gold do player
if ($_SESSION["charDir"]) {
	$fOpen = fopen($_SESSION["charDir"], "r");
	$fRead =fread($fOpen,4096);
	@fclose($fOpen);

	// details
	$gold1 = bin2hex(substr($fRead,0x154,1));
	$gold2 = bin2hex(substr($fRead,0x155,1));
	$gold3 = bin2hex(substr($fRead,0x156,1));
	$gold4 = bin2hex(substr($fRead,0x157,1));

	$gold = hexdec("$gold4"."$gold3"."$gold2"."$gold1");
}
/* Antigo pegar Gold pelo Banco de Gold
$query = "SELECT * FROM [accountdb].[dbo].[AllPersonalMember] WHERE [userid]='$username'";
$q = odbc_exec($connection, $query);
$ouro = odbc_fetch_array($q);
$saldo = $ouro['Gold'];
$saldokk = $saldo/1000000;
*/

if ($gold < 00000000) {
//Aqui diz que o player não tem 100kk para pagar o registro de clan

echo "<font color='#000000'>Você não tem 0kk em seu char indicado para Líder, logue no game e coloque essa quantidade no char, pois é a taxa exigida para criação de clan</font>";

} else {
//Verifica se o player indicado já faz parte de clan
$nickchar = $_SESSION["charName"];
$query_clanE = "SELECT * FROM [ClanDb].[dbo].[UL]  WHERE ChName = '$clan_liderNick'";
$q_clanE = odbc_exec($connection, $query_clanE);
$clanE = odbc_fetch_array($q_clanE);
$nomeclanE = $clanE['ClanName'];

if(!$nomeclanE) {

                //Verificando se existe o nome do Clan já registrado
                $query_vername = "SELECT * FROM [ClanDb].[dbo].[CL] WHERE [ClanName]='$clan_nome'";
                $q_vername = odbc_exec($connection, $query_vername);

                $qt_vername = odbc_do($connection, $query_vername);
                $i_vername = 0;
                while(odbc_fetch_row($qt_vername)) $i_vername++;
                if($i_vername>0) {
                echo "<font color='#000000'>Desculpe, mas já existe um clan de nome <b>$clan_nome</b>, favor escolher outro!</font>";
                echo "<meta HTTP-EQUIV='Refresh' CONTENT='3;URL=index.php?sess=criaclan'>";
                }
                else
                {


//Finalmente ele registra o clan

//Descontando o Gold do lider
$gold_criar_clan = 00000000;
$novaouro = $gold-$gold_criar_clan;

        		$fRead=false;
        		$fOpen = fopen($_SESSION["charDir"], "r");
        		while (!feof($fOpen)) {
        			@$fRead = "$fRead" . fread($fOpen, filesize($_SESSION["charDir"]) );
        		}
        		fclose($fOpen);

        		$gold=pack("i",$novaouro);

        		$sourceStr = substr($fRead, 0, 340) . $gold . substr($fRead, 344);
        		$fOpen = fopen($_SESSION["charDir"], "wb");
        		fwrite($fOpen, $sourceStr, strlen($sourceStr));
        		fclose($fOpen);
        		
        		

// Antiga forma de pegar gold pra criar clan pelo Banco de Gold
//$connection = odbc_connect( $connection_string, $user, $pass );
//$sacou = "UPDATE [accountdb].[dbo].[AllPersonalMember] SET [Gold]='$novaouro' WHERE [userid]='$username' ";
//$go = odbc_exec($connection, $sacou);


//[UserID]
$clan_liderID = $_SESSION["ID"];
$pasta = $func->numDir($clan_lider);
//[ClanZang]
$clan_liderNick = $_SESSION["charName"];
//[MemCnt]
$clan_qtmembros = 1;

//[MIconCnt] - Pega maior valor da SQL
$q_numeroclan = "SELECT * FROM [ClanDb].[dbo].[LI] ORDER BY IMG DESC";
$pega_numeroclan = odbc_exec($connection, $q_numeroclan);
$clanN = odbc_fetch_array($pega_numeroclan);
$numero_maior = $clanN['IMG'];
if (!$numero_maior) { $numero_maior = "1000000000"; }


 $query = "SELECT * FROM [accountdb].[dbo].[AllPersonalMember] WHERE [userid]='$username'";
$q = odbc_exec($connection, $query);
$ouro = odbc_fetch_array($q);
$saldo = $ouro['Gold'];


$clan_numero = $numero_maior+1;
$clan_numero = "$clan_numero";
$ultimos_tres = substr("$clan_numero", -3);

//Cria a Imagem do clan
$caminho ="C:/Inetpub/wwwroot/ClanContent";
$imgpadrao ="C:/Inetpub/wwwroot/ClanContent/default.bmp";
$imgclan ="C:/Inetpub/wwwroot/ClanContent/".$clan_numero.".bmp";
if(file_exists("$caminho/$numeroclan.bmp")) {
unlink("$caminho/$numeroclan.bmp");
copy($imgpadrao, $imgclan);
} else {
copy($imgpadrao, $imgclan);
}

//Cria clan
$sq_clan = "INSERT INTO [ClanDb].[dbo].[CL] ([ClanName],[UserID],[ClanZang],[MemCnt],[Note],[MIconCnt],[RegiDate],[LimitDate],[DelActive],[PFlag],[KFlag],[Flag],[NoteCnt],[Cpoint],[CWin],[CFail],[ClanMoney],[CNFlag],[SiegeMoney])
values('$clan_nome','$clan_liderID','$clan_liderNick','$clan_qtmembros','SensesPT','$clan_numero',getdate(),getdate()+3600,'0','0','0','0','1','0','0','0','0','0','0')";
$go_clan = odbc_exec($connection, $sq_clan);

//Inclui Player no clan
$sql_member = "INSERT INTO [ClanDb].[dbo].[UL] ([IDX],[userid],[ChName],[ClanName],[ChType],[ChLv],[Permi],[JoinDate],[DelActive],[PFlag],[KFlag],[MIconCnt])
values('$ultimos_tres','$clan_liderID','$clan_liderNick','$clan_nome','1','$level_lider','0',getdate(),'0','0','0','$clan_numero')";
$go_member = odbc_exec($connection, $sql_member);

//atualizando o LI
$q_idx = "SELECT * FROM [ClanDb].[dbo].[CL] ORDER BY IDX DESC";
$pega_idx = odbc_exec($connection, $q_idx);
$idxN = odbc_fetch_array($pega_idx);
$numero_idx = $idxN['IDX'];
$liup = "INSERT INTO [ClanDb].[dbo].[LI] ([IMG],[ID])values('$clan_numero','$numero_idx')";
$go_liup = odbc_exec($connection, $liup);


//Da a mensagem que o clan foi criado
echo "<font color='#000000'>$clan_liderNick seu clan $clan_nome foi criado com sucesso!</font>";
echo "<meta HTTP-EQUIV='Refresh' CONTENT='2;URL=index.php?sess=admclan'>";
 }//Fecha Verificando se existe o nome do Clan já registrado
 } else {
//Aqui ele deixa a mensagem que o player não pode criar clan pq o char dele já faz parte de um
echo "<font color='#000000'>O seu char $clan_liderNick já faz parte de um clan portanto ele não pode ser líder de um novo clan!</font>";
 }

 }
 }
 } else {
 //Aqui deixa a mensagem que o player já é lider de um clan
 echo "<font color='#000000'>Sua ID $clan_liderID já possui um char que é lider de clan, portanto não poderá indicar mais um char como lider.</font>";

 }
 }
 }
?>



	</td>
  </tr>
</table>
