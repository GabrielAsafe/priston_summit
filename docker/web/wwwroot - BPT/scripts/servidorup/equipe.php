<?php
ob_start( 'ob_gzhandler' );
require "config.php";
?>

<link href="<? echo $Website; ?>/css/scripts.css" rel="stylesheet" type="text/css">
<body style="background-color:transparent">
<tr>
<td>
<?
$gms_on = array();
$string = odbc_connect($Conexao_rPTDB, $Usuario_SQL, $Senha_SQL);
$query = "SELECT * FROM [ClanDB].[dbo].[CT]";
$q = odbc_exec($string, $query);
while($exe=odbc_fetch_array($q)){
$gms_on[] = $exe['ChName'];
}
$gm1 = "$membro1";
$gm2 = "$membro2";
$gm3 = "$membro3";
?>
<div id="equipe">
<ul>
<li>
<span class="equipe">
<? echo $gm1;?>
</span>
<ul>

<li>
<? if( (in_array($gm1,$gms_on)) <> false ){?> <img src="<? echo $Website_ServidorUP; ?>/imagens/online.png" width="47" height="11" alt="" border="0"><? } else { ?> <img src="<? echo $Website_ServidorUP; ?>/imagens/online.png" width="47" height="11" alt="" border="0"><? } ?>
</li>

</ul>
</li>


<li>
<span class="equipe">
<? echo $gm2;?>
</span>
<ul>

<li>
<? if( (in_array($gm2,$gms_on)) <> false ){?> <img src="<? echo $Website_ServidorUP; ?>/imagens/online.png" width="47" height="11" alt="" border="0"><? } else { ?> <img src="<? echo $Website_ServidorUP; ?>/imagens/online.png" width="47" height="11" alt="" border="0"><? } ?>
</li>

</ul>
</li>

<li>
<span class="equipe">
<? echo $gm3;?>
</span>
<ul>

<li>
<? if( (in_array($gm2,$gms_on)) <> false ){?> <img src="<? echo $Website_ServidorUP; ?>/imagens/online.png" width="47" height="11" alt="" border="0"><? } else { ?> <img src="<? echo $Website_ServidorUP; ?>/imagens/online.png" width="47" height="11" alt="" border="0"><? } ?>
</li>

</ul>
</li>
</div>

</tr>
</td>
</div>
</body>