<? if (PT!=1) exit; 
if(!$_POST['zera']){ ?>

<style type="text/css">
<!--
.fonte {
	font-family: Verdana, Geneva, sans-serif;
}
-->
</style>
</head>

<body>


	<table background="imgs/fundo_textura1.gif" width="600" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
                  <tr>
                    <td align="center" bgcolor="#000000"><b><font color="#FFFFFF">
					Sistema para resetar SOD</font></b></td>
                  </tr>
<tr>
<tr>
      <th width="728" height="38" scope="row"><span class="style2"><br>Zerando os pontos de SoD Clan</span></th>
    </tr>
	<form id="form" name="form" method="post" action="index.php?sessadm=zerarsod">
  <td width="59%"><p align="center">
    <label>
      <input type="submit" name="zera" id="zera" onclick ="return confirm('Você tem certeza que deseja, zerar a pontuação de sod?')" class="button6 value="Zerar" />
      <BR>

    </label>
  </p>
</form>
                  </tr>
                </table></td>
              </tr>
  </table>


<?
}elseif($_POST['zera']){
?>
<style type="text/css">
<!--
.fonte {
	font-family: Verdana, Geneva, sans-serif;
}
-->
</style>
</head>

<body>


	<table background="imgs/fundo_textura1.gif" width="600" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
                  <tr>
                    <td align="center" bgcolor="#000000"><b><font color="#FFFFFF">
					Sistema para resetar SOD</font></b></td>
                  </tr>
<tr>

<form id="form" name="form" method="post" action="index.php?sessadm=zerarsod">
  <td width="59%"><p align="center">
    <label>
<?
if($_SESSION['permissao'] < 3){
echo "Voce nao tem permissao para acessar esta area";
exit;
}
include("incluir/configura.php");

$pontos = 0;
$flag = 0;
$connection = odbc_connect( $connection_string, $user, $pass );
$query = "UPDATE [ClanDB].[dbo].[CL] SET [Cpoint]='$pontos' WHERE [Flag]='$flag' ";
$q = odbc_exec($connection, $query);
$query2 = "DELETE FROM [SoD2Db].[dbo].[SOD2RecBySandurr] WHERE [PMNo]='$flag' ";
$q = odbc_exec($connection, $query2);

if($query || $query2){
echo "Resetado com Sucesso!";
}else{
echo "Não pode ser resetado!";
}

?>

<?
}
?>