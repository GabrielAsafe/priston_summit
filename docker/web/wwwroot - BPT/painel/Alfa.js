//Verificando os campos do form
function verifica() {
	with(document.form_cad) {
		if(username.value.length < 4) {
			alert('Informe um Login com no mínimo 4 Digitos');
			username.focus()
			return false;
		}
		
		
//Função para Ajustar Apelido do Visitante
username.value = username.value.toLowerCase();
var estranha = "Ã¡Ã©Ã­Ã³ÃºÃ Ã¨Ã¬Ã²Ã¹Ã¢ÃªÃ®Ã´Ã»Ã¤Ã«Ã¯Ã¶Ã¼Ã£Ãµ@#$%^&*()_+=-~` Ã§¹²³£¢¬";
var correta  = "aeiouaeiouaeiouaeiouao________________c";
var retorno  = "";
for(i=0;i<estranha.length;i++)
   {
   for(j=0;j<username.value.length;j++)
      {
      retorno = username.value.replace(estranha.substr(i,1),correta.substr(i,1));
      retorno = retorno.replace("_","");
      username.value = retorno;
      }
   }
   
 
 		if(password.value.length < 4) {
			alert('É necessario informar uma Senha.');
			email.focus()
			return false;
		}
   		if (password.value != password2.value)
		{
		alert("As senhas estao diferentes. Verifique e tente novamente");
		email2.focus();
		return false;
		}
	
 		if(passwordp.value.length < 4) {
			alert('Informar uma Senha do painel.');
			passwordp.focus()
			return false;
		}
   		if (passwordp.value != passwordp2.value)
		{
		alert("As senhas do painel estao diferentes. Verifique e tente novamente");
		passwordps2.focus();
		return false;
		}


		if(nome.value.length < 4) {
			alert('Informe o seu PRIMEIRO NOME!');
			nome.focus()
			return false;
		}
		if(sobrenome.value.length < 3) {
			alert('Informe seu SOBRENOME!');
			sobrenome.focus()
			return false;
		}

        if (email.value.search(/^\w+((-\w+)|(\.\w+))*\@\w+((\.|-)\w+)*\.\w+$/) == -1) {
        alert("E-mail digitado incorretamente!");
        email.focus();
        return false;
        }

		if(email.value.length < 10) {
			alert('Informe o SEU E-MAIL.');
			email.focus()
			return false;
		}
		if (email.value != email2.value)
		{
		alert("Os E-mail estao diferentes. Verifique e tente novamente");
		email2.focus();
		return false;
		}


		if(cidade.value.length < 3) {
			alert('Informe o nome de sua CIDADE!');
			cidade.focus()
			return false;
		}
		if (estado.value == 0){
		alert("Informe SEU ESTADO!");
		estado.focus();
		return (false);
		}


		if(sexo.value == 0) {
			alert('Informes seu SEXO!');
			sexo.focus()
			return false;
		}


		if (nasc_dia.value == 0){
		alert("Informe o DIA DO SEU NASCIMENTO!");
		nasc_dia.focus();
		return (false);
		}
		
		if (nasc_mes.value == 0){
		alert("Informe o MÃŠS DO SEU NASCIMENTO!");
		nasc_mes.focus();
		return (false);
		}
		
		if (nasc_ano.value == 0){
		alert("Informe o ANO DO SEU NASCIMENTO!");
		nasc_ano.focus();
		return (false);
		}
		
		
		if (perg.value == 0){
		alert("Informe uma pergunta de segurança");
		perg.focus();
		return (false);
		}	
		if(resp.value.length < 1) {
			alert('Informe uma resposta a pergunta de segurança!');
			resp.focus()
			return false;
		}

}}

//Verifica campos de atualização
function verifica2() {
	with(document.form_cad) {

 		if(chaveregistro.value.length < 10) {
			alert('É necessario informar a Chave de Alteração');
			chaveregistro.focus()
			return false;
		}
		
 		if(oldpwd.value.length < 4) {
			alert('É necessario informar a Senha Atual.');
			oldpwd.focus()
			return false;
		}

 		if(pwd1.value.length < 4) {
			alert('É necessario informar uma Senha.');
			pwd1.focus()
			return false;
		}
   		if (pwd1.value != pwd2.value)
		{
		alert("As senhas estao diferentes. Verifique e tente novamente");
		pwd2.focus();
		return false;
		}


		if(nome.value.length < 4) {
			alert('Informe o seu PRIMEIRO NOME!');
			nome.focus()
			return false;
		}
		if(sobrenome.value.length < 3) {
			alert('Informe seu SOBRENOME!');
			sobrenome.focus()
			return false;
		}

        if (email.value.search(/^\w+((-\w+)|(\.\w+))*\@\w+((\.|-)\w+)*\.\w+$/) == -1) {
        alert("E-mail digitado incorretamente!");
        email.focus();
        return false;
        }

		if(email.value.length < 10) {
			alert('Informe o SEU E-MAIL.');
			email.focus()
			return false;
		}
		if (email.value != email2.value)
		{
		alert("Os E-mail estao diferentes. Verifique e tente novamente");
		email2.focus();
		return false;
		}


		if(cidade.value.length < 3) {
			alert('Informe o nome de sua CIDADE!');
			cidade.focus()
			return false;
		}
		if (estado.value == 0){
		alert("Informe SEU ESTADO!");
		estado.focus();
		return (false);
		}


		if(sexo.value == 0) {
			alert('Informes seu SEXO!');
			sexo.focus()
			return false;
		}


		if (nasc_dia.value == 0){
		alert("Informe o DIA DO SEU NASCIMENTO!");
		nasc_dia.focus();
		return (false);
		}

		if (nasc_mes.value == 0){
		alert("Informe o MÃŠS DO SEU NASCIMENTO!");
		nasc_mes.focus();
		return (false);
		}

		if (nasc_ano.value == 0){
		alert("Informe o ANO DO SEU NASCIMENTO!");
		nasc_ano.focus();
		return (false);
		}
}}


//Verifica campos de atualização
function verificaCP() {
	with(document.form_cad) {

        if(oldpwd.value.length < 4) {
			alert('É necessario informar a Senha Atual.');
			oldpwd.focus()
			return false;
		}

		if(nome.value.length < 4) {
			alert('Informe o seu PRIMEIRO NOME!');
			nome.focus()
			return false;
		}
		if(sobrenome.value.length < 3) {
			alert('Informe seu SOBRENOME!');
			sobrenome.focus()
			return false;
		}

        if (email.value.search(/^\w+((-\w+)|(\.\w+))*\@\w+((\.|-)\w+)*\.\w+$/) == -1) {
        alert("E-mail digitado incorretamente!");
        email.focus();
        return false;
        }

		if(email.value.length < 10) {
			alert('Informe o SEU E-MAIL.');
			email.focus()
			return false;
		}
		if (email.value != email2.value)
		{
		alert("Os E-mail estao diferentes. Verifique e tente novamente");
		email2.focus();
		return false;
		}


		if(cidade.value.length < 3) {
			alert('Informe o nome de sua CIDADE!');
			cidade.focus()
			return false;
		}
		if (estado.value == 0){
		alert("Informe SEU ESTADO!");
		estado.focus();
		return (false);
		}


		if(sexo.value == 0) {
			alert('Informes seu SEXO!');
			sexo.focus()
			return false;
		}


		if (nasc_dia.value == 0){
		alert("Informe o DIA DO SEU NASCIMENTO!");
		nasc_dia.focus();
		return (false);
		}

		if (nasc_mes.value == 0){
		alert("Informe o MES DO SEU NASCIMENTO!");
		nasc_mes.focus();
		return (false);
		}

		if (nasc_ano.value == 0){
		alert("Informe o ANO DO SEU NASCIMENTO!");
		nasc_ano.focus();
		return (false);
		}
}}

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