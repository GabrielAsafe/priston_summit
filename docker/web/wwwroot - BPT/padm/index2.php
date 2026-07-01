<?
if (PT!=1) exit;
$get = $_GET['sessadm'];
if($get == "deslogar"){
	session_unregister("gmfodoes");
	unset($_SESSION["IDADM"]);
	unset($_SESSION["NICKGM"]);
	header("Location: logout.php");
}
header('Content-Type: text/html; charset=iso-8859-1');
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<html>
<head>
<title><?=$version?></title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<link href="css/style.css" rel="stylesheet" type="text/css">
<style type="text/css">
<!--
.vermelho {	color: #F00;
}
.style1 {color: #FF0000}
.style2 {color: #0000FF}
-->
</style>
</head>

<body background="imgs/fundo_cinza_conteudos.jpg" leftmargin="0" topmargin="0" marginwidth="0" marginheight="0" allowtransparency="true">
<table width="800" border="1" align="center" cellpadding="0" cellspacing="0" bordercolor="#333333">
  <tr>
    <td>
      <table width="800" border="0" align="center" cellpadding="0" cellspacing="0">
        <tr>
          <td><img src="imgs/banner.gif"></td>
        </tr>
      </table>
      <table width="800" bgcolor="#FFFFFF" border="1" bordercolor="#333333" cellpadding="0" cellspacing="0">
        <tr>
        <td>

            <table width="100%" border="0" align="center" cellpadding="4" cellspacing="0">

        <tr>
          <td align="right"><table width="100%" border="0" cellspacing="0" cellpadding="0">
            <tr></tr>
            <tr>
              <td height="25" valign="middle"><p align="left">Ol&aacute; , <strong><? echo $_SESSION["NICKGM"]; ?></strong> Seja bem vindo ao Painel ADMINISTRADOR SummitTale<br>
                <a href="index.php?sessadm=deslogar" onClick="return confirm('tem certeza que deseja deslogar do Painel de Administra&ccedil;&atilde;o?');"><span class="style2"><strong>DESLOGAR DO PAINEL</strong></span></a><span class="style1"><a href="http://189.8.42.29/paineladm/index.php?sessadm=deslogar" onClick="return confirm('tem certeza que deseja deslogar do Painel de Administra&ccedil;&atilde;o?');"></a></span>              <a href="index.php?sessadm=deslogar" onClick="return confirm('tem certeza que deseja deslogar do Painel de Administra&ccedil;&atilde;o?');"><br>
                <br>
              </a></p></td>
            </tr>
            <tr>
              <td height="25" align="center" valign="middle"><p align="center"><strong><img src="imgs/Suporte.png" width="150" height="20"><br>
                <br>
              </strong><a href="index.php?sessadm=logsID">VIA ID </a>|  <a href="index.php?sessadm=logsIP">VIA IP</a> |  <a href="index.php?sessadm=logsCOD">VIA CODIGO ITEM</a> | <a href="index.php?sessadm=procuraritem">PROCURAR ITEMS</a> | </a><a href="index.php?sessadm=dados"> CHARS NA CONTA </a>| <a href="index.php?sessadm=status">STATUS DA CONTA</a><br>
              <br>
              <strong><img src="imgs/Servidor.png" width="150" height="20"></strong><br>
                  <strong><br>
                  </strong><a href="index.php?sessadm=zerarank">ZERAR RANK</a> | <a href="index.php?sessadm=deletaitemlog">ZERAR ITEMLOGDB</a> | <a href="index.php?sessadm=zerarct">ZERAR CT</a> | <a href="index.php?sessadm=zerarsod">ZERAR SOD CLAN</a> | <a href="index.php?sessadm=resetsod">ZERAR SOD PLAYER</a> | <a href="index.php?sessadm=clan">DELETAR CLAN</a> |  <a href="index.php?sessadm=deletaclans">DELETAR CLANS</a> | <a href="index.php?sessadm=deletarconta">DELETA CONTA </a><br>
                  <br>
                  </a><strong><img src="imgs/admin.png" width="150" height="20"><br>
                    <br>
                    </strong><a href="index.php?sessadm=backup">BACKUP SQL</a> | <a href="index.php?sessadm=bugdat">VERIFICAR DAT</a> <a href="index.php?sessadm=addgoldsod">|</a>  <a href="index.php?sessadm=ligar">LIGAR SERVIDOR</a> | </a><a href="index.php?sessadm=level"> UP PLAYER</a><a href="index.php?sessadm=addgoldsod"> |</a><a href="index.php?sessadm=addgold"> BANCO VIRTUAL</a><br>
                    <a href="index.php?sessadm=addgoldp">ADD GOLD AO CHAR </a><a href="index.php?sessadm=addgoldsod">|</a><a href="index.php?sessadm=addgoldp"> </a> <a href="index.php?sessadm=addgoldbc"> GOLD BC CLAN</a> | </a><a href="index.php?sessadm=addgoldsod"> GOLD SOD CLAN</a> | <a href="index.php?sessadm=moverchar"> MOVER CHAR</a><a href="index.php?sessadm=addgoldsod"> |</a><a href="index.php?sessadm=teleport"> TELEPORTAR CHAR </a><a href="index.php?sessadm=addgoldsod">|</a> <a href="index.php?sessadm=multimanager"> ATUALIZAR RANKING </a><br><a href="index.php?sessadm=hotuk">EDITAR HOTUK</a> | <a href="index.php?sessadm=uon">USU&Aacute;RIOS ONLINE</a> | <a href="index.php?sessadm=banned">EDITAR EXCALIBUR</a></br> 
                    <br>
                    <strong><font color="#008000"><img src="imgs/Player.png" width="150" height="20"></font></strong> <br>
                  <br>
                  </strong><a href="index.php?sessadm=search">VER  ID PELO NICK </a> | </a><a href="index.php?sessadm=contas">DADOS PLAYER </a> | </a><a href="index.php?sessadm=banir">PUNIR PLAYER</a> | <a href="index.php?sessadm=listaban">LISTA PUNIDOS </a> | </a><a href="index.php?sessadm=banidosdoserver">LISTA BANIDOS </a> | </a><a href="index.php?sessadm=mudarnick">MUDAR NICK | </a><a href="index.php?sessadm=mudarclasse">MUDAR CLASSE</a></br>
				  <br>
                    <strong><font color="#008000"><img src="imgs/shop.png" width="150" height="20"></font></strong> <br>
                  <br>
				  <a href="index.php?sessadm=informacao">OBTER INFORMA&Ccedil;&Otilde;ES</a> |<a href="index.php?sessadm=addcredito">ADICIONAR CR&Eacute;DITO </a><a href="index.php?sessadm=addgoldsod">| </a><a href="index.php?sessadm=removecredito">REMOVER CR&Eacute;DITO</a><a href="index.php?sessadm=addgoldsod"> | <a href="index.php?sessadm=editarextrato">EDITAR EXTRATO </a></br> 
				  <br>
                    <strong><font color="#008000"><img src="imgs/Premiacao.png" width="150" height="20"></font></strong> <br>
                  <br>
				  <a href="index.php?sessadm=premiado"> PREMIAR USER ON </a>| <a href="index.php?sessadm=premiados">LISTA DE PREMIADOS</a> | <a href="index.php?sessadm=distribuidor"> ITEM DISTRIBUIDOR </a>| <a href="index.php?sessadm=log_dis"> LOG ITEM DISTRIBUIDOR </a>| <a href="index.php?sessadm=zerarlog_dis"> ZERAR LOG ITEM DISTRIBUIDOR </a>
                </p></td>
            </tr>
            <tr>
              <td height="25" align="center" valign="middle">&nbsp;</td>
            </tr>
          </table></td>
        </tr>
        <tr>
        <td>
<?
$sessadm=(!$_GET[sessadm])?"char":$_GET[sessadm];
switch($sessadm)
{

   case "banir":
        include_once "banir.inc.php";
    break;

    case "listaban":
        include_once "punidos.inc.php";
    break;

    case "home":
        include_once "home.inc.php";
    break;
	
	    case "level":
        include_once "level.inc.php";
    break;
	
	    case "zerarrank":
        include_once "zerarrank.inc.php";
    break;
	
	case "contas":
        include_once "contas.inc.php";
    break;

	case "logsID":
        include_once "logID.inc.php";
    break;
	
		case "recover":
        include_once "restore.inc.php";
    break;

		case "recovercabelo":
        include_once "recovercabelo.inc.php";
    break;
	
			case "moverchar":
        include_once "moverchar.inc.php";
    break;

     case "contas_2":
    include_once "contas_2.inc.php";
    break;
	
			case "teleport":
        include_once "teleport.inc.php";
    break;

			case "multimanager":
        include_once "multimanager.inc.php";
    break;
	
			case "addgoldp":
        include_once "addgoldp.inc.php";
    break;
		
	
	case "logsIP":
        include_once "logIP.inc.php";
    break;
	
	
	case "logsCOD":
        include_once "logCOD.inc.php";
    break;
	
	case "search":
        include_once "search.inc.php";
    break;
	
		case "deletaitemlog":
        include_once "deletaitemlog.inc.php";
    break;
	
	case "resetsod":
        include_once "resetsodplayer.inc.php";
    break;

	case "deposita":
        include_once "depositar.inc.php";
    break;

	case "addgold":
        include_once "addgold.inc.php";
    break;
	
		case "depositabc":
        include_once "depositarbc.inc.php";
    break;

	case "addgoldbc":
        include_once "addgoldbc.inc.php";
    break;
	
			case "depositasod":
        include_once "depositarsod.inc.php";
    break;

	case "addgoldsod":
        include_once "addgoldsod.inc.php";
    break;


	case "resetsod":
        include_once "resetsod.inc.php";
    break;
	
	case "mudarnick":
        include_once "mudarnick.inc.php";
    break;

	case "mudarclasse":
        include_once "mudarclasse.inc.php";
    break;
	
		case "deletaclans":
        include_once "deletaclans.inc.php";
    break;
		
	
	case "ligar":
        include_once "ligarservidor.inc.php";
    break;

	case "bugdat":
        include_once "bugdat.inc.php";
    break;

	case "adados":
        include_once "adados.php";
    break;
		
	case "ligarsv":
        include_once "completa_ligarservidor.inc.php";
    break;
	
		case "clan":
        include_once "clan.inc.php";
    break;
	
	case"addcredito":
		  include_once"addcredito.inc.php";
		  break;
		  
		   case"removecredito":
		  include_once"removecredito.inc.php";
		  break;
	
			case "zerarct":
        include_once "zerarct.inc.php";
    break;

	
	case "delclan":
        include_once "delclan.inc.php";
    break;
	
		case "procuraritem":
        include_once "procuraritem.inc.php";
    break;
	
	case "itemresultado":
        include_once "itemresultado.inc.php";
    break;

	case "banidosdoserver":
        include_once "banidosdoserver.php";
    break;

	case "allcontas":
        include_once "allcontas.inc.php";
    break;

	case "deletarconta":
        include_once "deletarconta.inc.php";
    break;

	case "delconta":
        include_once "delconta.inc.php";
    break;
	
	case "hotuk":
		include "hotuk/index.php";
	break;
	
	case "hotukedit":
		include "hotuk/editar.php";
	break;

	case "uon":
		include "uon.php";
	break;
	
	case "informacao":
        include_once "informacao.php";
    break;
	
	case "editarextrato":
		include "edita_extrato.php";
	break;
	case "premiado":
        include_once "premiado.php";
    break;
	
	case "distribuidor":
        include_once "distribuidor.inc.php";
    break;
	
	case "premiados":
        include_once "lista_permiados.php";
    break;
	
	case "backup":
		include "backup.php";
	break;
	
	case "log_dis":
		include "log_distribuidor.php";
	break;
	
	case "dados":
        include_once "dados.inc.php";
    break;
	
	case "status":
        include_once "statuscontas.inc.php";
    break;
	
	case "zerarank":
        include_once "zerarank.php";
    break;
	
	case "banned":
		include "excalibur/index.php";
	break;
	
	case "bannededit":
		include "excalibur/editar.php";
	break;

	case "zerarsod":
		include "zerarsod.php";
	break;
	
	case "zerarlog_dis":
		include "zerarlogdistribuidor.php";
	break;
}
?>



		</td>
</tr>
        <tr>
          <td align="center"><p><strong>---------------------------------------------------------------------------------------------------------------------------------------------------------------<br>
                  <span class="style1"></span> Painel de Administração
                  <span class="style2"></span> do Servidor<br></p>
            </td>
        </tr>
      </table>

	    </td>
        </tr>
      </table>

  </td>
</tr>

</table>

</body>
</html>




