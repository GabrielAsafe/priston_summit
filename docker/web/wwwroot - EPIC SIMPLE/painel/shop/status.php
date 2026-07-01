<?
$extencao = ".png";
if(isset($_GET['item']) && ($_GET['item'] != "")){
$imagem = "status/".$_GET['item']."".$extencao."";
if(file_exists($imagem)){
?>
<body bgcolor="#3b291d">
<img src="status/<? echo $_GET['item'];?><? echo $extencao;?>" height="399">	
<?		
}else{
	echo "<img src='status/sem_status.png' height='399'>";
	//echo "<b><font color=red><center>IMG indisponivel</center></font></b>";						
}
}else{
exit();	
}
?>
</body>