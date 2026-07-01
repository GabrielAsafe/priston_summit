<?php
ob_start( 'ob_gzhandler' );
require "config.php";
?>

<link href="<? echo $Website; ?>/css/scripts.css" rel="stylesheet" type="text/css">
<body style="background-color:transparent" background="<? echo $Website; ?>/imagens/fundoonline.png">
<tr>
<td>

<?php
$servidor = @fsockopen("69.162.107.21", $porta, $errno, $errstr, $ativar);
if($servidor >= 1){
echo '<iframe src ="totalonline.php" scrolling="No" frameborder="0" marginwidth="0" marginheight="0" vspace="0" hspace="0" allowtransparency="true" width="203" height="26"><p>Sem suporte iFrame</p></iframe>';}
else{ echo '<div id="serverstatus">Este servidor está <font color="#FF0000">desligado</font></div>';}
?>

</tr>
</td>
</body>