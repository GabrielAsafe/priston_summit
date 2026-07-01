<?php
ob_start('ob_gzhandler');
session_start();

header('p3p: CP="CAO PSA OUR"');

include_once "incluir/configura.php";
include_once "injection.php";
include_once "verifica_tempo.php";

if ($_GET['sess'] == "logout") {
    session_destroy();
    header("location: index.php");
    exit;
}

function escapestrings($string) {
    if (!get_magic_quotes_gpc()) {
        return mysql_escape_string($string);
    } else {
        return $string;
    }
}

if (!$_SESSION["ID"]) {
?>
<!DOCTYPE HTML>
<html lang="pt-BR">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Summit Priston Tale</title>

<link rel="shortcut icon" href="assets/images/gt_favicon2.png">
<link rel="stylesheet" media="screen" href="http://fonts.googleapis.com/css?family=Open+Sans:300,400,700">
<link rel="stylesheet" href="assets/css/bootstrap.min.css">
<link rel="stylesheet" href="assets/css/font-awesome.min.css">
<link rel="stylesheet" href="assets/css/bootstrap-theme.css" media="screen" >
<link rel="stylesheet" href="assets/css/main.css">

<style>
html, body {
    height: 100%;
    margin: 0;
}
body {
    background: url('imgs/background1.jpg') no-repeat center center fixed;
    background-size: cover;
    display: flex;
    align-items: center;
    justify-content: center;
    position: relative;
    font-family: "Open Sans", sans-serif;
}
.overlay {
    position: absolute;
    top: 0; left: 0; right: 0; bottom: 0;
    background: rgba(0, 0, 0, 0.55);
    z-index: 1;
}
.login-box {
    background: rgba(255, 255, 255, 0.95);
    padding: 25px;
    border-radius: 8px;
    width: 100%;
    max-width: 400px;
    z-index: 2;
    box-shadow: 0 0 20px rgba(0,0,0,0.4);
}
.login-box h3 {
    text-align: center;
    margin-bottom: 20px;
    font-weight: bold;
}
</style>

<script>
document.addEventListener('contextmenu', e => e.preventDefault());
function disabledBttn(formname) {
    if (document.all || document.getElementById) {
        for (let i=0;i<formname.length;i++) {
            var bttn=formname.elements[i];
            if(bttn.type.toLowerCase()=="submit" || bttn.type.toLowerCase()=="reset" || bttn.type.toLowerCase()=="button")
                bttn.disabled=true;
        }
    }
}
</script>

</head>
<body>
<div class="overlay"></div>
<div class="login-box">
<?php
if ($_POST['action'] != "Logar") {
    require 'gerarrand.php';
    $strRand = gerar_string_randonica();
?>  
    <h3>Painel Summit Tale</h3>
    <form method="post" onSubmit="disabledBttn(this)" action="index.php">
        <div class="form-group">
            <label>ID: <span class="text-danger">*</span></label>
            <input name="username" type="text" class="form-control">
        </div>
        <div class="form-group">
            <label>Senha: <span class="text-danger">*</span></label>
            <input name="password" type="password" class="form-control">
        </div>
        <div class="row">
            <div class="col-xs-6">
                <a href="recsenha.php"><b>Esqueceu sua senha?</b></a>
            </div>
            <div class="col-xs-6 text-right">
                <a href="cadtermos.php"><b>Criar conta</b></a>
            </div>
        </div>
        <br>
        <button class="btn btn-primary btn-block" name="action" type="submit">ENTRAR</button>
        <input type="hidden" name="action" value="Logar">
    </form>
<?php
} else {
    $required=array(
        "Seu ID" => $_POST['username'],
        "Sua Senha" => $_POST['password'],
    );

    $usuarioAPT = anti_sqli(escapestrings($_POST['username']));
    $senhaAPT = anti_sqli(escapestrings($_POST['password']));
    $usuarioAPT = trim($usuarioAPT);
    $senhaAPT = trim($senhaAPT);

    if (anti_sql($usuarioAPT) != 0 || anti_sql($senhaAPT) != 0) {
        echo "<meta HTTP-EQUIV='Refresh' CONTENT='0;URL=index.php'>";
    } else {
        foreach ($required as $key=>$value) {
            if (!$value) echo "Você esqueceu de informar <b>$key</b>.<br>";
            else $chkArr[]=true;
        }

        if (count($chkArr) == count($required)) {
            $connection = odbc_connect($connection_string, $user, $pass);

            $querysenha = "SELECT * FROM [accountdb].[dbo].[AllPersonalMember] WHERE [Userid]='$usuarioAPT'";
            $qtsenha = odbc_do($connection, $querysenha);
            $i2 = 0;
            while(odbc_fetch_row($qtsenha)) $i2++;

            if ($i2 > 0) {
                $usernameP = anti_sqli($_POST['username']);
                $query = "SELECT * FROM [accountdb].[dbo].[".( strtoupper($usernameP[0]) ) ."GameUser] 
                          WHERE [userid]='$usuarioAPT' AND [Passwd]='$senhaAPT'";
                $qt = odbc_do($connection, $query);
                $i = 0;
                while(odbc_fetch_row($qt)) $i++;

                if ($i > 0) {
                    session_register("usercode");
                    $_SESSION["ID"] = $usuarioAPT;
                    echo "<div class='text-center'><b>Carregando...</b><br><img src='imgs/loading.gif' width='20' height='20'></div>";
                } else {
                    echo "<div class='text-center'><b>ID ou Senha incorretos!</b><br><img src='imgs/loading.gif' width='20' height='20'></div>";
                }
            } else {
                echo "<div class='text-center'><b>ID ou Senha incorretos!</b><br><img src='imgs/loading.gif' width='20' height='20'></div>";
            }

            echo "<meta HTTP-EQUIV='Refresh' CONTENT='2;URL=index.php'>";
        }
    }
}
?>
</div>
</body>
</html>
<?php
exit;
}
require "index2.php";
ob_end_flush();
?>
