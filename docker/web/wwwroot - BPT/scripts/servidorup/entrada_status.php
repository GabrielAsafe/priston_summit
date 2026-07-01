<?php
ob_start( 'ob_gzhandler' );
require "config.php";
?>

<link href="<? echo $Website; ?>/css/scripts.css" rel="stylesheet" type="text/css">
<body style="background-color:transparent">
<tr>
<td>

<?php
$servidor = @fsockopen("124.198.128.16", $porta, $errno, $errstr, $ativar);
if($servidor >= 1){
echo '<div align="center"><img src="'.$Website.'/imagens/entrada_16.png" width="154" height="13" alt="" border="0"></div>';}
else{ echo '<div align="center"><img src="'.$Website.'/imagens/entrada_16_16.png" width="154" height="13" alt="" border="0"></div>';}
?>

</tr>
</td>
</body>