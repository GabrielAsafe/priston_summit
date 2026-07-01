<?if (PT!=1) exit;?>

<?

<?php
include_once "injection.php";
include_once "incluir/configura.php";

$hotuk = "hotuk.ini";
$file = "$rootDir2$hotuk";
$post = stripslashes($_POST['hotuk']);
$fp = file_put_contents($file, $post);

if($fp){
echo "
<script>
alert(\"Hotuk editada com sucesso!\");
</script>
";
echo "<script>history.go(-1);</script>";
}else{
echo "
<script>
alert(\"Falha!\");
</script>
";
echo "<script>history.go(-1);</script>";
}
?>