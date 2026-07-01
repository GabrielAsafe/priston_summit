<?
	function gerar_string_randonica()
	{
		$caractmin  = array("a","b","c","d","e","f","g","h","i","j","l","m","n","o","p","q","r","s","t","u","v","x","z","w");
		$caractmax  = array("A","B","C","D","E","F","G","H","I","J","L","M","N","O","P","Q","R","S","T","U","V","X","Z","W");
		$numeral    = array("0","1","2","3","4","5","6","7","8","9");
		$keycaracti = array_rand($caractmin,2);
		$keycaractx = array_rand($caractmax,2);
		$keynumeral = array_rand($numeral,2);

		$stringfinal = $caractmin[$keycaracti[0]].$numeral[$keynumeral[0]].$caractmax[$keycaractx[0]].
    		           $caractmin[$keycaracti[1]].$numeral[$keynumeral[1]].$caractmax[$keycaractx[1]].
					   $caractmin[$keycaracti[2]].$numeral[$keynumeral[2]].$caractmax[$keycaractx[2]];
		$stringfinal = base64_encode($stringfinal);
		return $stringfinal;
	}
?>