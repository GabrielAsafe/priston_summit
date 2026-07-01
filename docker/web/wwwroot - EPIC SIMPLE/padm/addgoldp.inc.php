<?if (PT!=1) exit;?>

<?
include_once "incluir/configura.php";
include_once "injection.php";
?>
</style>
<link href="css/style.css" rel="stylesheet" type="text/css">
<table background="imgs/fundo_textura1.gif">
              
                  
<link href="style.css" rel="stylesheet" type="text/css">
<body background="background.gif">
<form id="form1" name="form1" method="post" action="">
<table background="imgs/fundo_textura1.gif" width="712" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td width="712"><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
                  <tr>
                    <td colspan="2" align="center" bgcolor="#000000"><b><font color="#FFFFFF">Adicionar Gold direto ao Char</span></th>
    </tr>
<table background="imgs/fundo_textura1.gif" width="716" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td width="716"><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
<form id="form1" name="form1" method="post" action="">
  <table width="550" border="0">
    <tr>
      <td width="120">Adicionar gold a um char</td>
      <td width="420">&nbsp;</td>
    </tr>
    <tr>
      <td>Nick do char</td>
      <td><label>
        <input type="text" name="nick" id="nick" />
      </label></td>
    </tr>
    <tr>
      <td>Gold a adicionar</td>
      <td><label>
        <input type="text" name="gold" id="gold" />
      </label></td>
    </tr>
    <tr>
      <td>&nbsp;</td>
      <td><label>
        <input type="submit" name="button" class="button6" id="button" value="Adicionar" />
        <input name="acao" type="hidden"  class="button6" id="acao" value="addgold" />
      </label></td>
    </tr>
 
<tr>
                  <td colspan="2" align="center"><div align="left">
                    <p><strong>Instru&ccedil;&otilde;es de como Utilizar o Sistema de Adicionar gold no inventario do player.<br />
  </strong>1 Passo - Digite o nick do player que voc&ecirc; deseja mandar o gold o personagem.<br />
2 Passo - Em GOLD ADICIONAR coloque a quantidade de gold que deseja mandar para o personagem dele.<br />
<strong>Exemplos</strong>:1000000= 1kk, 10000000=10kk, 50000000=50kk,100000000=100kk<br />
3 Passo - Clique em adicionar, vai falar que o gold foi enviado com sucesso para o player.<br> Maximo de gold que pode enviar, 500000000=500kk </p>
                     </table>
                  
      </table>
<table width="300" border="0">
  <tr>
    <td><?
	if($_POST['acao'] == "addgold"){
	$gold = $_POST['gold'];
	$nick = $_POST['nick'];
	if(empty($gold) && empty($nick)){
	echo"<b><font color='red'>preencha todos os campos!!";
	}else{
	
	$pasta = $dirUserData . ( $func->numDir($nick)) . "/" . $nick . ".dat";
	
	$_POST['gold']=$func->getInt($_POST['gold']);

            if( $_POST['gold']<=500000000)
            {
                $fRead=false;
                $fOpen = fopen($pasta, "r");
                while (!feof($fOpen)) {
                @$fRead = "$fRead" . fread($fOpen, filesize($pasta) );
                }
                fclose($fOpen);

                $gold=pack("i",$_POST['gold']);

                $sourceStr = substr($fRead, 0, 340) . $gold . substr($fRead, 344);
                $fOpen = fopen($pasta, "wb"); 
                fwrite($fOpen, $sourceStr, strlen($sourceStr));
                fclose($fOpen);


                echo "<b><font color='green'>você adicionou <u> ".$_POST['gold']."</u>em gold ao char $nick!";
            }
            else
            {
                echo "<b><font color='red'><center>DINHEIRO MAXIMO PERMITIDO eh de 500000000!";
            }

	
	
	
	
	}
	}
	
	
	
	
	?></td>
  </tr>
</table>
</body>
</html>
