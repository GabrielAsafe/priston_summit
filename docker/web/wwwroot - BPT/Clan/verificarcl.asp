<!-- #include file ="settings.asp" -->
<%
	Dim dbhost, dbuser, dbpass, dbname, userid, gserver, chname, clName, expl, chtype, lv

	FillSettings()
	 
	Dim strSplit
	strSplit = Chr("&H" & "0D")
	
	userid = Trim(Request("userid"))
	gserver = Trim(Request("gserver"))
	chname = Trim(Request("chname"))
	clName = Trim(Request("clName"))
	chtype = Trim(Request("chtype"))
	lv = Trim(Request("lv"))
		 
	expl = "Priston Tale"
	 
	if userid="" Or gserver="" Or chname="" Or clName="" Or chtype="" or lv="" Then
		 Response.Write("Code=100" & strSplit)
		 Response.End
	End if
	
	Dim QUERY, QUERY_VER, RS, RS_VER, objConn
	
	Set objConn = Server.CreateObject("ADODB.Connection")
	objConn.Open "Provider=SQLOLEDB; Data Source=" & dbhost & "; Initial Catalog=" & dbname & "; User ID=" & dbuser & "; Password=" & dbpass
	
	Set RS = Server.CreateObject("ADODB.Recordset")
	Set RS_VER = Server.CreateObject("ADODB.Recordset")

	QUERY_VER = "SELECT * FROM CT WHERE userid='" & userid & "'"
	RS_VER.Open QUERY_VER, objConn, 3, 1

	if RS_VER.RecordCount = 1 Then
		
		QUERY = "SELECT ClanName FROM UL WHERE ChName='" & chname & "'"
		RS.Open QUERY, objConn, 3, 1
	
		if RS.RecordCount > 0 Then
			If RS("ClanName").Value = "" Then
				QUERY = "DELETE FROM UL WHERE ChName='" & chname & "'"
				RS.Open QUERY, objConn, 3, 1
			Else
				Set RS = Nothing
				objConn.Close
				Set objConn = Nothing
				strReturn = "Code=2" & strSplit & "CMoney=0" & strSplit
				Response.Write(strReturn)
				Response.End
			End If
		End if
		Rs.Close()
	
		QUERY = "SELECT ClanZang FROM CL WHERE ClanName='" & clName & "'"
		RS.Open QUERY, objConn, 3, 1
		
		if RS.RecordCount > 0 Then
			Set RS = Nothing
			objConn.Close
			Set objConn = Nothing
			strReturn = "Code=3" & strSplit & "CMoney=0" & strSplit
			Response.Write(strReturn)
			Response.End
		End if
		Rs.Close()
		
		QUERY = "SELECT IMG FROM LI WHERE ID=1"
		RS.Open QUERY, objConn, 3, 1
		 
		Dim iIMG
		
		if RS.RecordCount > 0 Then
			iIMG = RS("IMG").Value
	
			Rs.Close()
		Else
			
			Rs.Close()
			
			iIMG = 1000000000
	
			QUERY = "INSERT INTO LI values('" & (iIMG + 1) & "','1')"
			RS.Open QUERY, objConn, 3, 1
			Rs.Close()
		End If
		 
		iIMG = iIMG + 1
	
		QUERY = "UPDATE LI SET IMG='" & iIMG & "' WHERE ID=1"
		RS.Open QUERY, objConn, 3, 1
		
		QUERY = "INSERT INTO CL ([ClanName],[UserID],[ClanZang],[MemCnt],[Note],[MIconCnt],[RegiDate],[LimitDate],[DelActive],[PFlag],[KFlag],[Flag],[NoteCnt],[Cpoint],[CWin],[CFail],[ClanMoney],[CNFlag],[SiegeMoney]) values('" & clName & "','" & userid & "','" & chname & "','1','" & expl & "','" & iIMG & "',getdate(),getdate()+3600,'0','0','0','0','1','0','0','0','0','0','0')"
		RS.Open QUERY, objConn, 3, 1
	
		QUERY = "SELECT IDX FROM CL WHERE ClanName='" & clName & "'"
		RS.Open QUERY, objConn, 3, 1
		
		if RS.RecordCount > 0 Then
			IDX = RS("IDX").Value
		End If
		Rs.Close()
	
			
		QUERY = "INSERT INTO UL ([IDX],[userid],[ChName],[ClanName],[ChType],[ChLv],[Permi],[JoinDate],[DelActive],[PFlag],[KFlag],[MIconCnt]) values('" & IDX & "','" & userid & "','" & chname & "','" & clName & "','" & chtype & "','" & lv & "','0',getdate(),'0','0','0','" & iIMG & "')"
		RS.Open QUERY, objConn, 3, 1
		
		strReturn = "Code=1" & strSplit & "CMoney=500000" & strSplit
		SET RS = Nothing
		objConn.Close
		SET objConn = Nothing
	
		Response.Write(strReturn)
	
else

	Response.Write("voc&ecirc; est&aacute; tentando criar o clan pelo navegador.")
	
end if

	Set RS_VER  = Nothing
%>
