<?if (PT!=1) exit;?>
<?
/*
WebSistema Criado por Jeck
*/

include_once "injection.php";
include_once "incluir/configura.php";

$connection = odbc_connect($connection_string, $user, $pass) or die ("Erro ao se conectar com o SQL Server");

function AcertarLinhasODBC($conn,$tabela){
	$resultado = odbc_exec($conn,$tabela);
	$contador=0;
	while($temp = odbc_fetch_into($resultado, &$counter)){
		$contador++;
	}
	return $contador;
}


$total = "SELECT * FROM [ADM].[dbo].[LogsDistribuidor]";
$contagemtotal= AcertarLinhasODBC($connection, $total);


?>

<table width="448" border="0" align="center" cellpadding="5" cellspacing="0">
  <tr>
    <td>    
      <br>
      <table width="100%" border="0" cellspacing="0" cellpadding="0">
        <tr>
          <td width="50%"><b><font color="#006600"><img src="imgs/moedas.gif" width="25" height="16"> Total
                De Sorteados:</font></b> <? echo $contagemtotal; ?> </td>
				
         
        </tr>
      </table>
      <br>
      <table width="100%" border="0" cellpadding="2" cellspacing="1" bordercolor="#333333">
        <tr>
          <td colspan="5" align="center" bgcolor="#000000"><font color="#FFFFFF"><strong>Lista de Sorteados</strong></td>
        </tr>
        <tr>
          <td align="center" bgcolor=""><strong>nick</strong></td>
          <td align="center" bgcolor=""><strong>data</strong></td>
          <td align="center" bgcolor="">id</td>
          <td align="center" bgcolor=""><strong>item</strong></td>
           </tr>
		<?
		$conta = "SELECT * FROM [ADM].[dbo].[LogsDistribuidor]";
$contaa = odbc_exec($connection, $conta);

while($exe= odbc_fetch_array($contaa)){

{
       
		$nick= $exe['nick'];
		$data= $exe['data'];
		$id= $exe['id'];
		$item = $exe['item'];
	
		?>
        <tr>
          <td align="center" bgcolor="#FFFFFF"><? echo $nick; ?></td>
          <td align="center" bgcolor="#FFFFFF"><? echo $data; ?> </td>
          <td align="center" bgcolor="#FFFFFF"><? echo $id;  ?></td>
          <td align="center" bgcolor="#FFFFFF"><? echo $item; ?>  </td>
          
		   
        </tr> 
<? } } ?>
</table></td>
  </tr>
  <tr>
    <td>&nbsp;</td>
  </tr>
</table>




