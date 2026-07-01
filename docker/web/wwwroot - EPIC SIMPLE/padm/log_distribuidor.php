<? if (PT!=1) exit; ?>
<?php
include_once "incluir/configura.php";

$connection = odbc_connect($connection_string, $user, $pass) or die ("Erro ao se conectar com o SQL Server");

?>
<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#000000">
<tr>
<td align="center" bgcolor="#000000"><font color="#FFFFFF"> GM </font></td>
<td align="center" bgcolor="#000000"><font color="#FFFFFF"> Item </font></td>
<td align="center" bgcolor="#000000"><font color="#FFFFFF"> Char </font></td>
<td align="center" bgcolor="#000000"><font color="#FFFFFF"> Data </font></td>
<td align="center" bgcolor="#000000"><font color="#FFFFFF"> ID Char </font></td>
<?
				//Verificando se Existe essa ID
                $query = "SELECT * FROM [ADM].[dbo].[LogsDistribuidor] ORDER BY data DESC";
                $q = odbc_exec($connection, $query);
				$qt = odbc_do($connection, $query);
                $i = 0;
                while(odbc_fetch_row($qt)) {
				$farr = odbc_fetch_array($q);
				$nickGM=$farr[idGM];
				$item=$farr[item];
				$nick=$farr[nick];
				$data=$farr[data];
				$id=$farr[id];
?>
</tr>
<tr>
<td align="center"><? echo $nickGM; ?>&nbsp;</td>
<td align="center"><? echo $item; ?>&nbsp;</td>
<td align="center"><? echo $nick; ?>&nbsp;</td>
<td align="center"><? echo $data ?>&nbsp;</td>
<td align="center"><? echo $id; ?>&nbsp;</td>
</tr>
<? } ?>
</table>

