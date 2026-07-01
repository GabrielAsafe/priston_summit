<?if (PT!=1) exit;?><br>
<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#330099">
<tr>
<td align="center" bgcolor="#000000"><font color="#FFFFFF">ID do Player</font></td>
<td align="center" bgcolor="#000000"><font color="#FFFFFF">Motivo</font></td>
<td align="center" bgcolor="#000000"><font color="#FFFFFF">Puni&ccedil;&atilde;o</font></td>
<td align="center" bgcolor="#000000"><font color="#FFFFFF">Data Ban </font></td>
<td align="center" bgcolor="#000000"><font color="#FFFFFF">GM</font></td>
<?
				//Verificando se Existe essa ID
				$connection = odbc_connect( $connection_string, $user, $pass );
                $query = "SELECT * FROM [ADM].[dbo].[LogsBan] ORDER BY data DESC";
                $q = odbc_exec($connection, $query);
				$qt = odbc_do($connection, $query);
                $i = 0;
                while(odbc_fetch_row($qt)) {
				$farr = odbc_fetch_array($q);
				$idplayer=$farr[idplayer];
				$motivo=$farr[motivo];
				$punicao=$farr[punicao];
				$diaban=$farr[diaban];
				$mesban=$farr[mesban];
				$anoban=$farr[anoban];
				$diadesban=$farr[diadesban];
				$mesdesban=$farr[mesdesban];
				$anodesban=$farr[anodesban];
				$gm=$farr[gm];

?>
</tr>
<tr>
<td align="center"><? echo $idplayer; ?>&nbsp;</td>
<td><? echo $motivo; ?>&nbsp;</td>
<td align="center"><? echo $punicao; ?>&nbsp;</td>
<td align="center"><? echo "$diaban/$mesban/$anoban"; ?>&nbsp;</td>
<td align="center"><? echo $gm; ?>&nbsp;</td>
</tr>
<? } ?>
</table><br>

