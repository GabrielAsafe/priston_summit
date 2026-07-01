<?php
if (PT!=1) exit;
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
					Sistema para editar extrato</font></b></td>
                  </tr>
<tr>

<form id="form" name="form" method="post" action="index.php?sessadm=editarextrato&acao=editar">
  <td width="59%">                       
     <p align="center"><strong><font color="#000000">ID</font></strong>                       
     <input type="text" name="id" id="id" />
     </p>
     <p align="center">
       <label>
      <input type="submit" name="Alterar" id="Alterar" class="button6" value="Enviar" />
	  
<BR><BR>

    </label>
  </p>
</form>
                  </tr>
                </table></td>
              </tr>
  </table>

<?
}elseif($_GET['acao'] == 'editar'){
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
					Sistema para editar extrato</font></b></td>
                  </tr>
<tr>

<form id="form" name="form" method="post" action="index.php?sessadm=editarextrato&acao=gravar&id=<?php echo $_POST['id']; ?>">
  <td width="59%">                       
     <p align="center">
       <label>
         <?php
	   $file = "$extrato".$_POST['id'].".his";
	   if(!file_exists($file)){
		   echo 'ID inexistente!';
		   exit;
	   }
	   $fp = fopen($file, "r");
	   $fr = @fread($fp, filesize($file));
	   
	   echo 'ID: <strong>'.$_POST['id'].'</strong><br />';
	   ?>
         <br>
         <textarea name="extrato" id="extrato" cols="80" rows="20"><?php echo $fr; ?></textarea>
         <br>
         <br>
         <input type="submit" name="Editar" id="Editar" class="button6" value="Editar">
         <BR>
         <BR>
         
       </label>
     </p>
  </form>
                  </tr>
                </table></td>
              </tr>
  </table>
<?
}elseif($_GET['acao'] == 'gravar'){
?>
<style type="text/css">
<!--
.fonte {
	font-family: Verdana, Geneva, sans-serif;
}
-->
</style>

	<table background="imgs/fundo_textura1.gif" width="600" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
                  <tr>
                    <td align="center" bgcolor="#000000"><b><font color="#FFFFFF">
					Sistema para editar extrato</font></b></td>
                  </tr>
<tr>

  <td width="59%"><p align="center">
    <label>
<?php

$id = $_GET['id'];
if(anti_sql($id) != 0){
echo "Caracteres inválidos!";
exit;
}

$file = "$extrato".$_GET['id'].".his";
if(!file_exists($file)){
echo 'ID inexistente!';
exit;
}

$fp = fopen($file, "w");
$fw = fwrite($fp, stripslashes($_POST['extrato']));

if($fw){
echo 'Extrato editado com sucesso!';
}else{
echo 'Ocorreu um erro, tente novamente!';
}

?><br>
      <br>
      <a href="index.php?sessadm=editarextrato">Voltar</a><BR>
         <BR>
         
       </label>
     </p>
                  </tr>
                </table></td>
              </tr>
  </table>
<?
}
?>
