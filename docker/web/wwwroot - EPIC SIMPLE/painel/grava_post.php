<? if (Painel!=1) exit;
/*----------------------------------------------------------------------
Painel Player Versão 0.4
Desenvolvidor Por: Mak (sirmakloud@gmail.com)
Editado Por: Jiraya (richard.cva21@hotmail.com(
----------------------------------------------------------------------*/
?>
<?
if(isset($_SESSION['passou'])){
$_SESSION['cancelar'] = "YES";
unset($_SESSION['passou']);
header("Location: index.php?sess=cancelaclean");
}else{
header("Location: index.php?sess=bau");	
}
?>