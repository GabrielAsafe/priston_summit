<?php
if (empty($_SESSION["ID"]) || empty($_SESSION["tempo_permitido"])) {
    return;
}

$tempo_permitido = (int) $_SESSION["tempo_permitido"];
$tempo_atual = time();

if ($tempo_atual > $tempo_permitido) {
    session_destroy();
    header("Location: index.php?sess=logout");
    exit;
}
?>
