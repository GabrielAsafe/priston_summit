<?if (PT!=1)  exit;?>
</style>
<link href="css/style.css" rel="stylesheet" type="text/css">
<table background="imgs/fundo_textura1.gif">
              
                  
<link href="style.css" rel="stylesheet" type="text/css">
<body background="background.gif">
<form id="form1" name="form1" method="post" action="">
<table background="imgs/fundo_textura1.gif" width="712" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td width="712"><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
                  <tr>
                    <td colspan="2" align="center" bgcolor="#000000"><b><font color="#FFFFFF">Sistema de Atualização do Rank</span></th>
    </tr>
<?
/* --------------------------------------------------------------
Script Atualiza ranking via php
By D-KiN (ks2002br@yahoo.com.br) 
Data criação: 19/10/2009
Modificações:
20/10/2009 By D-KiN

------------------------------------------------------------------*/
?>
<?
include_once "injection.php";
include_once "incluir/configura.php";
?>

<HTML>
<HEAD>
<TITLE>Atualiza Rank PhP</TITLE>
<meta http-equiv="refresh" content="180";>   
</HEAD>
<BODY>

<?
$dirUserData = "".$rootDir."DataServer/userdata/";  //Diretorio do Server de PT

$connection1 = odbc_connect( $connection_string, $user, $pass );

$query1 = "DELETE [rPTDB].[dbo].[LevelList]";
$q1 = odbc_exec($connection1, $query1);
?>
  <table border="1" align="center" cellpadding="0" cellspacing="0">
  <tr bgcolor="#ffffff">
  <td width="50%"><div align="center"><strong><font size="2" face="Verdana, Arial, Helvetica, sans-serif"><bgcolor="#ffffff">Dados Gravados no SQL</font></strong></div></td>

  <table border="1" align="center" cellpadding="0" cellspacing="0">
     
    <tr bgcolor="#CCCCCC">
	<td width="50%"><div align="center"><strong><font size="2" face="Verdana, Arial, Helvetica, sans-serif">Classe</font></strong></div></td>
    <td width="38%"><div align="center"><strong><font size="2" face="Verdana, Arial, Helvetica, sans-serif">Char</font></strong></div></td>
	<td width="40%"><div align="center"><strong><font size="2" face="Verdana, Arial, Helvetica, sans-serif">Level</font></strong></div></td>
   </tr>

<?
$narq =  count(scan_Dir($dirUserData));
$charDat = scan_Dir($dirUserData);
$total = 0;

for($i=0; $i<$narq; $i++) {
?>
    <tr>
        <td><?         
			  
     	$fOpen = fopen($charDat[$i], "r");                 // Abre o Arquivo .dat
        $fRead =fread($fOpen,filesize($charDat[$i]));      // Le o arquivo .dat
        @fclose($fOpen);                                   // Fecha o arquivo .dat
		
		$charName = trim(substr($fRead,0x10,15),"\x00");  // Pega o Nome do char no offset 0x10
        $charClass =  substr($fRead,0xc4,1);              // Pega o codigo da Classe do char no offset 0xc4
		$charLevel =   substr($fRead,0xc8,1);             // Pega o codigo do Nivel do Char
        $charLevel = ord($charLevel);                     // Converte o Nivel do Char para decimal (ascii)
        			
		      switch (ord($charClass))           // Chavea o valor da classe convertido para decimal (ascii)
                {
                  case 1: $class = 'Lutador';   break;     //Se a chave for 1 é Lutador
                  case 2: $class = 'Mecanico';   break;    //Se a chave for 2 é Mecanico
                  case 3: $class = 'Arqueira';   break;    //Se a chave for 2 é Mecanico
                  case 4: $class = 'Pikeman';   break;     //Se a chave for 2 é Mecanico
                  case 5: $class = 'Atalanta';   break;    //Se a chave for 2 é Mecanico 
                  case 6: $class = 'Cavaleiro';   break;   //Se a chave for 2 é Mecanico 
                  case 7: $class = 'Mago';   break;        //Se a chave for 2 é Mecanico
                  case 8: $class = 'Sacerdotiza';   break; //Se a chave for 2 é Mecanico
                }
	    $charClass = $class		

?>
	 
    <tr>
      <td align="center" bgcolor="#FFFFFF"><? echo $charClass; ?></td>
      <td align="center" bgcolor="#FFFFFF"><? echo $charName; ?></td>
      <td align="center" bgcolor="#FFFFFF"><? echo $charLevel; ?> </td>
  	
<?	
       	   
       $connection = odbc_connect( $connection_string, $user, $pass );
       $query = "INSERT INTO [rPTDB].[dbo].[LevelList] ([CharName],[CharLevel],[CharClass],[ID]) values('$charName','$charLevel','$charClass','1')";
       $q = odbc_exec($connection, $query);
 
?>

<?
}
function scan_Dir($dir) {
   $arrfiles = array();
   if (is_dir($dir)) {
       if ($handle = opendir($dir)) {
           chdir($dir);
           while (false !== ($file = readdir($handle))) {
               if ($file != "." && $file != "..") {
                   if (is_dir($file)) {
                       $arr = scan_Dir($file);
                       foreach ($arr as $value) {
                           $arrfiles[] = $dir."/".$value;
                       }
                   } else {
                       $arrfiles[] = $dir."/".$file;
                   }
               }
           }
           chdir("../");
       }
       closedir($handle);
   }
   return $arrfiles;
}
 	
?>
</table>  </table>

  <table border="1" align="center" cellpadding="0" cellspacing="0">
    <tr>
	<td width="80%" align="center" bgcolor="#FFCC00"><strong>Foram lidos: <? echo $i  ?> Dats </strong></td><tr>
    <tr><td width="80%" align="center" bgcolor="#FFCCAA"> Ultima Leitura as: <strong><? echo $today = date("j/F/Y - g:i a");?></strong></td>
<tr><td width="10%" align="center" bgcolor="#FFCCAA"> Desenvolvido por D-KiN: </td>
   </table>
   
<?
exit;
?>    	

</BODY>
</HTML>
