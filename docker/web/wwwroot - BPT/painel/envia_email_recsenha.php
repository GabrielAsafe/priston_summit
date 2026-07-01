<?
//faz chamada das classes na pasta libs
require("smtp/class.phpmailer.php");
require("incluir/configura.php");

$mail = new PHPMailer();
$mail->SetLanguage("br", "smtp/"); //Idioma
$mail->IsSMTP();

$mail->SMTPAuth = true;
$mail->Host = "endereço do servidor smtp"; //Host do servidor SMTP
$mail->Username = "o seu e-mail smtp da hospedagem ou free"; //Nome do usuario SMTP
$mail->Password = "a senha do e-mail";     //Senha do usuario SMTP
$mail->From = "o seu e-mail smtp da hospedagem ou free"; //Email de quem envia
$mail->FromName = "$nomedoserver";    //nome de quem ta enviando, vai aparecer na coluna "De:"

//INICIO --- Quem vai receber-----------------------------------------------
$mail->AddAddress("".$email."");
//FIM --- Quem vai receber--------------------------------------------------

$mail->AddReplyTo("o seu e-mail smtp da hospedagem ou free"); //Quem irá receber a resposta (quando a pessoal responder)
$mail->IsHTML(true);

//Assunto
$mail->Subject = "Sua senha";
//Corpo da mensagem, pode usar tags html
$mail->Body = "Ol&aacute;, ".$array['nome']." foi solicitado uma recupera&ccedil;&atilde;o de senha da conta, <b>".$array['Userid']."</b><br><br>
Login: ".$array['Userid']."<br>
Senha: ".$array['Passwd']."<br><br>

Se n&atilde;o foi voc&ecirc; quem solicitou o pedido restaura&ccedil;&atilde;o de senha, iguinore esta mensagem e fique tranquilo pois a senha da sua conta s&oacute; pode ser enviada para o e-mail de cadastro.<br><br>

<b>Atenciosamente, Equipe $nomedoserver</b><br>
<a href='www.AlfaPT.com.br'>www.AlfaPT.com.br</a>
";

if(!$mail->Send())
{
echo "A mensagem não pode ser enviada. <p>";
echo "Erro do envio: " . $mail->ErrorInfo;
exit;
}

?>