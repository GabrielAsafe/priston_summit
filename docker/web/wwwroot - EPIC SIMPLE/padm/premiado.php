<? if (PT!=1) exit; 
if($_POST['acao'] == ''){?>

<form id="form" name="form" method="post">
	<table background="imgs/fundo_textura1.gif" width="600" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
                  
          <tr> 
            <td width="63%" align="center" bgcolor="#000000"><b><font color="#FFFFFF">Sistema Logado Premiado</font></b></td>
                  </tr>
                 <tr>
      <th width="721" scope="row"><span class="style2">Premiação para quem está online</span></th>
    </tr>
            <td align="center"><input type="submit" class="button6" onclick ="return confirm('Deseja sortear agora?')" value="Premiar" />
              <input name="acao" type="hidden" id="acao" value="del"></td>
                  </tr>
                </table></td>
              </tr>
  </table>
           </form>
<?
}else{
?>
<form id="form" name="form" method="post">
	<table background="imgs/fundo_textura1.gif" width="600" border="0" align="center" cellpadding="0" cellspacing="0">
              <tr>
                <td><table width="100%" border="0" align="center" cellpadding="4" cellspacing="2">
                  
          <tr> 
            <td width="63%" align="center" bgcolor="#000000"><b><font color="#FFFFFF">Sistema para Premiar Usuario ON</font></b></td>
                  </tr>
                  <tr>
                  
            <td align="center">
<?
if($_SESSION['permissao'] < 3){
echo "Você não tem permissão para acessar esta área";
exit;
}
/**************************************************************************
WebSistema Criado por: Jeck(jeckzinho@hotmail.com) Respeite os Créditos!  *
***************************************************************************/

if ($HTTP_SERVER_VARS["HTTP_X_FORWARDED_FOR"] != "") {
        echo "VOCÊ ESTÁ USANDO PROXY SAFADO!";
}

include_once "incluir/configura.php";

$quantidade = "24";  // Quantidade de Itens Diarios

$connection_string = 'DRIVER={SQL Server};SERVER='.$host.';DATABASE=ClanDB'; //NOME PC

// USUARIO DO SQLEXPRESS USER E PASSWORD 

$connection = odbc_connect($connection_string, $user, $pass);

$query = odbc_exec($connection, "SELECT * FROM CT ORDER BY rand()");

while(odbc_fetch_array($query)) // odbc_fetch_row, ou array o que vc gosta mais
{
    $usuarios[] = odbc_result($query,"ChName");
}
// sortiar
$procura = $usuarios[ (rand(0,count($usuarios))) ];

$dirUserData = "".$rootDir."/DataServer/userdata/";

function subDiretorio($pasta)
{
 	$total = 0;
	for($i=0;$i<strlen($pasta);$i++)
	{			
		$total += ord(strtoupper($pasta[$i]));
			if($total >= 256)
			{
				$total = $total - 256;
			}
		
	}
	return $total;
}

$procuraPlayer = "".$rootDir."/DataServer/Userdata/".subDiretorio($procura).'/'.$procura.'.dat';

	// lê o arquivo para descobrir a ID
	$aberto = @fopen($procuraPlayer, "r");
	$conteudoDat =@fread($aberto,filesize($procuraPlayer));
	@fclose($aberto);

	$PlayerID = trim(substr($conteudoDat,0x000002C0,15),"\x00");
	 $data = date("d-m-Y");
	 $hora = date("h:i:s");
	 
	
	 function AcertarLinhasODBC($conexao,$query){
		$resultado = odbc_exec($conexao, $query);
		$contador=0;
		while($temp = odbc_fetch_into($resultado, $counter)){
			$contador++;
		}
		return $contador;
	}
	
$contaitens = "SELECT * FROM [Jeck].dbo.[Premiado] WHERE Data like '$data'";



$contagem= AcertarLinhasODBC($connection, $contaitens);

    //VERIFICA SE O USUÁRIO DIGITADO EXISTE E SE ELE CRIOU A POUCO TEMPO
 if( ( $quantidade == $contagem) ){

	                     echo"<b>Jah foi Sorteado $quantidade Itens Hoje!";
					
	}else{
	
	
	
	 
 $query = "SELECT * FROM [Jeck].[dbo].[premiado] WHERE [id]='$PlayerID' AND [DATA]='$data'";
                $q = odbc_exec($connection, $query);

                $qt = odbc_do($connection, $query);
                $i = 0;
                while(odbc_fetch_row($qt)) $i++;

                if($i>0){
                    echo"<b>O Usuario <font color='#FF0000'>$procura</font> ja recebeu um Premio Hoje!!";
	}else{
	
				
$queryitem = odbc_exec($connection, "SELECT * FROM [Jeck].[dbo].[itens] ORDER BY rand()");

while(odbc_fetch_array($queryitem)) // odbc_fetch_row, ou array o que vc gosta mais
{
    $itens[] = odbc_result($queryitem,"Codigo");
}
// sortiar
$sorteiaitem = $itens[ (rand(0,count($itens))) ];

//Dados do Item

$dados_item = "$procura $sorteiaitem \"Parabéns Você Foi Sorteado no SnowPT\"". "\r\n";

$pasta_entrega = "".$rootDir."/PostBox/".subDiretorio($PlayerID)."/".$PlayerID.".dat";


$query_3 = "SELECT * FROM [Jeck].[dbo].[itens] WHERE [CODIGO]='$sorteiaitem'";
$qt = odbc_do($connection,$query_3);
while(odbc_fetch_row($qt)){
	$itemname = odbc_result($qt,2);
	$i =1;
}


$query_4 = "INSERT INTO [Jeck].[dbo].[premiado] ([ID],[Nick],[DATA],[HORA],[PREMIO]) VALUES ('$PlayerID','$procura','$data','$hora','$itemname')";


	$q_4 = odbc_exec($connection, $query_4);

    if ($q_4){
	
echo " O Item enviado foi $sorteiaitem para o ID: $PlayerID cujo o Nick é : $procura";
echo "<br> Obrigado por usar meu sistema *-*";
//Enviando o Item para o Distribuidor
if (file_exists($pasta_entrega)) {
$fp = fopen($pasta_entrega, "a+");
//Escreve o pedido
$escreve = fwrite($fp, "$dados_item");
// Fecha o arquivo
fclose($fp);
} else {
copy("shop.dat",$pasta_entrega);
$fp = fopen($pasta_entrega, "r+");
//Escreve o pedido
$escreve = fwrite($fp, "$dados_item");
// Fecha o arquivo
fclose($fp);    }
            }
        }
    } 

?>
            </td>
                  </tr>
                </table></td>
              </tr>
  </table>
           </form>
<?
}
?>