//Verificando os campos do form
function verifica() {
	with(document.form_cad) {
		if(bannick.value.length < 4) {
			alert('Informe o nick do Player a ser Banido');
			bannick.focus()
			return false;
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
