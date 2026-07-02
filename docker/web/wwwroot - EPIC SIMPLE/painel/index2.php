<? if (Painel!=1) exit;
?>
<?
  include_once "injection.php";
?>
<!DOCTYPE html>
<html lang="pt-br">

<head>
  <meta charset="utf-8">
  <meta http-equiv="X-UA-Compatible" content="IE=edge">
  <meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
  <meta name="description" content="">
  <meta name="author" content="">
  <title>Painel do Jogador</title>
  <!-- Bootstrap core CSS-->
  <link href="vendor/bootstrap/css/bootstrap.min.css" rel="stylesheet">
  <!-- Custom fonts for this template-->
  <link href="vendor/font-awesome/css/font-awesome.min.css" rel="stylesheet" type="text/css">
  <!-- Page level plugin CSS-->
  <link href="vendor/datatables/dataTables.bootstrap4.css" rel="stylesheet">
  <link rel="shortcut icon" href="assets/images/gt_favicon.png">
  <!-- Custom styles for this template-->
  <link href="css/sb-admin.css" rel="stylesheet">
  <link rel="stylesheet" href="css/main.css">
  <script language="JavaScript" type="text/JavaScript">
<!--
function disabledBttn(formname)
{
    if (document.all || document.getElementById) {
        for (i=0;i<formname.length;i++) {
            var bttn=formname.elements[i];
            if(bttn.type.toLowerCase()=="submit" || bttn.type.toLowerCase()=="reset" || bttn.type.toLowerCase()=="button")
                bttn.disabled=true;
        }
    }
}
//-->
function click() {
if (event.button==2||event.button==3) {
oncontextmenu='return false';
  }
}
document.onmousedown=click
document.oncontextmenu = new Function("return false;")

</script>
<script language="JavaScript" type="text/JavaScript">
<!--
function disabledBttn(formname)
{
    if (document.all || document.getElementById) {
        for (i=0;i<formname.length;i++) {
            var bttn=formname.elements[i];
            if(bttn.type.toLowerCase()=="submit" || bttn.type.toLowerCase()=="reset" || bttn.type.toLowerCase()=="button")
                bttn.disabled=true;
        }
    }
}
//-->
</script>
</head>
  <?
    $username=$_SESSION["ID"];
    include'incluir/configura.php';
    $conecta = odbc_connect( $connection_string, $user, $pass );
    $query = "SELECT * FROM [ClanDb].[dbo].[CT] WHERE [UserID]='$username'";
    $executa = odbc_exec($conecta,$query);
    while($busca_resulta = odbc_fetch_array($executa)){
    $id_online = $busca_resulta['UserID'];
    }
    if($id_online == $username){
    header("location: playeron.php");
    }else{
    }
    ?>
    <?
    $username=$_SESSION["ID"];
    $conecta = odbc_connect( $connection_string, $user, $pass );
    $query = "SELECT * FROM [accountdb].[dbo].[AllGameUser] WHERE [userid]='$username'";
    $executa = odbc_exec($conecta,$query);
    while($busca_resulta = odbc_fetch_array($executa)){
    $banido = $busca_resulta['BlockChk'];
    }
    if($banido == "1"){
    header("location: banido.php");
    }else{
    }
  ?>
<body class="fixed-nav sticky-footer bg-dark" id="page-top">
  <!-- Navigation-->
  <nav class="navbar navbar-expand-lg navbar-dark bg-dark fixed-top" id="mainNav">
    <a class="navbar-brand" href="?sess=inicial">Menu do Jogador</a>
    <button class="navbar-toggler navbar-toggler-right" type="button" data-toggle="collapse" data-target="#navbarResponsive" aria-controls="navbarResponsive" aria-expanded="false" aria-label="Toggle navigation">
      <span class="navbar-toggler-icon"></span>
    </button>
    <div class="collapse navbar-collapse" id="navbarResponsive">
      <ul class="navbar-nav navbar-sidenav" id="exampleAccordion">
        <li class="nav-item" data-toggle="tooltip" data-placement="right" title="chars">
          <a class="nav-link nav-link-collapse collapsed" data-toggle="collapse" href="#collapseChars" data-parent="#exampleAccordion">
            <i class="fa fa-fw fa-lg fa-user"></i>
            <span class="nav-link-text" > Personagens</span>
          </a>
          <ul class="sidenav-second-level collapse" id="collapseChars">
            <li>
              <a href="https://drive.google.com/file/d/1caixHlbe_6XogrEY0YgVNT6_t3SRY6MP/view?usp=drive_link">Download Client Full</a>
            </li>
             <li>
              <a href="https://www.mediafire.com/file/y3aatez34brj5fw/game.rar/file">Update Obrigatorio</a>
            </li>
            <li>
              <a href="https://discord.gg/9JbQsDD74X">Discord</a>
            </li>
            <li>
              <a href="?sess=inicial">Criar char ingame</a>
            </li>
          </ul>
        </li>
        

      </ul>


      <ul class="navbar-nav ml-auto">
        <div class="user">
          Olá, 
          <input type="hidden" name="username" value="<?=$_SESSION["ID"]?>">
                        <?=$_SESSION["ID"]?></td>
        </div>
      
        <li class="nav-item">
          <a class="nav-link" href="?sess=logout">
            <i class="fa fa-fw fa-sign-out"></i>Sair</a>
        </li>
      </ul>
    </div>
  </nav>
  <div class="content-wrapper">
    <div class="container-fluid">
        <div class="row">
          <?
            $sess=(!$_GET[sess])?"inicial":$_GET[sess];
            switch($sess)
            {
                case "mudarnick":
                    include_once "classe/nick.inc.php";
                break;
              
                    case "rank1":
                    ($_SESSION["charDir"])?include_once "rank/rank1.php":"";
                break;
              
                    case "rank2":
                    ($_SESSION["charDir"])?include_once "rank/rank2.php":"";
                break;

                case "mudarclasse":
                    include_once "classe/classe.inc.php";
                break;

                case "inicial":
                    include_once "inicial.inc.php";
                break;

                case "criarchar":
                    include_once "char.inc.php";
                break;
              
              case "bau":
                    include_once "bau/bau.inc.php";
                break;
              
              case "clan":
                    include_once "clan/cria.inc.php";
                break;
              
                case "criarclan":
                    include_once "clan/criar_clan.inc.php";
                break;
              
              case "sairclan":
                   include_once $rootPainel."clan/sairclan.inc.php";
                break;
              
                case "skill":
                    ($_SESSION["charDir"])?include_once "skill.inc.php":"";
                break;

                case "state":
                    ($_SESSION["charDir"])?include_once "state.inc.php":"";
                break;

                case "mudasenha":
                    include_once "atualizar.inc.php";
                break;
              
              
                //Administra dados do Clan e exibe infos dele
                case "admclan":
                    include_once "clan/admclan.inc.php";
                break;

                    //Lider vê membros do clan
                case "membros":
                    include_once "clan/membrosdoclan.inc.php";
                break;

              //Shop
                case "shop":
              include_once "shop/principal.php";
                break;

                case "compra":
                    include_once "shop/compra.inc.php";
                break;

                case "extrato":
                    include_once "shop/extrato.inc.php";
                break;

                case "donate":
                    include_once "shop/donate.php";
                break;

               case "armas":
            $arquivo = "shop/itens/armas.ite";
            $nomecat = "armas";
            include_once "shop/itens.inc.php";
              break;
              
              case "armaduras":
            $arquivo = "shop/itens/armaduras.ite";
            $nomecat = "Armaduras e Robes";
            include_once "shop/itens.inc.php";
              break;
              
              case "escudos":
            $arquivo = "shop/itens/escudos.ite";
            $nomecat = "Escudos e Orbitais";
            include_once "shop/itens.inc.php";
              break;

            case "sets":
            $arquivo = "shop/itens/sets.ite";
            $nomecat = "Braceletes, luvas e botas";
            include_once "shop/itens.inc.php";
              break;
              
              case "aneis":
            $arquivo = "shop/itens/aneis.ite";
            $nomecat = "Aneis e amuletos";
            include_once "shop/itens.inc.php";
              break;
              
              case "age":
            $arquivo = "shop/itens/sheltons.ite";
            $nomecat = "Age e Mix";
            include_once "shop/itens.inc.php";
              break; 

            case "itenspremium":
            $arquivo = "shop/itens/premium.ite";
            $nomecat = "Hunting e PvP";
            include_once "shop/itens.inc.php";
              break; 

            }
            ?>
        </div>
      </div>
    </div>
    <!-- /.container-fluid-->
    <!-- /.content-wrapper-->
    <footer class="sticky-footer">
      <div class="container">
        <div class="text-center">
          <small>Copyright © Summit Priston Tale</small>
        </div>
      </div>
    </footer>
    <!-- Scroll to Top Button-->
    <a class="scroll-to-top rounded" href="#page-top">
      <i class="fa fa-angle-up"></i>
    </a>
    <!-- Logout Modal-->
   
    <!-- Bootstrap core JavaScript-->
    <script src="vendor/jquery/jquery.min.js"></script>
    <script src="vendor/bootstrap/js/bootstrap.bundle.min.js"></script>
    <!-- Core plugin JavaScript-->
    <script src="vendor/jquery-easing/jquery.easing.min.js"></script>
    <!-- Page level plugin JavaScript-->
    <script src="vendor/chart.js/Chart.min.js"></script>
    <script src="vendor/datatables/jquery.dataTables.js"></script>
    <script src="vendor/datatables/dataTables.bootstrap4.js"></script>
    <!-- Custom scripts for all pages-->
    <script src="js/sb-admin.min.js"></script>
    <!-- Custom scripts for this page-->
    <script src="js/sb-admin-datatables.min.js"></script>
    
  </div>
</body>

</html>
