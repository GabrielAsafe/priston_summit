<? if (PT!=1) exit; 
if(!$_POST['bk']){ ?>
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
					Sistema para backup do SQL SERVER</font></b></td>
                  </tr>
<tr>

<form id="form" name="form" method="post">
  <td width="59%">                       
     <p align="center">
       <label>
         <input type="submit" name="bk" id="bk" class="button6" value="Backup" />
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
}else{
if($_SESSION['permissao'] < 3){
echo "Você não tem permissão para acessar esta área";
exit;
}
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
					Sistema para backup do SQL SERVER</font></b></td>
                  </tr>
<tr>

<form id="form" name="form" method="post">
  <td width="59%">                       
     <p align="center">
       <label><br />
         <?php

$lugar = "backup.bat";
$abre = exec($lugar);
if(!$abre){
	echo 'Backup realizado com sucesso';
}

?><BR>
         <BR>
         
       </label>
     </p>
  </form>
                  </tr>
                </table></td>
              </tr>
  </table>

<?
}
?>


