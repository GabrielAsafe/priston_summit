<!-- #include file ="settings.asp" -->
<%
'Written By Sandurr COPYRIGHT Sandurr 2006
'Written By Sandurr COPYRIGHT Sandurr 2006
'Written By Sandurr COPYRIGHT Sandurr 2006
'Written By Sandurr COPYRIGHT Sandurr 2006
'Written By Sandurr COPYRIGHT Sandurr 2006
'Version 2.0 NOVEMBER 2006

' Assign Global Variables
Dim dbhost, dbuser, dbpass, dbname, userid, gserver, chname, clName

FillSettings()

Dim strSplit
strSplit = Chr("&H" & "0D")

' Parameter Variables
' clanRemove (userid, gserver, chname, clName)
userid = Trim(Request("userid"))
gserver = Trim(Request("gserver"))
chname = Trim(Request("chname"))
clName = Trim(Request("clName"))


if userid = "" Or gserver = "" Or chname = "" Or clName = "" Then
Response.Write("Code=100" & strSplit)
Response.End
End if

Dim QUERY, QUERY_VER, RS, RS_VER, objConn

Set objConn = Server.CreateObject("ADODB.Connection")
objConn.Open "Provider=SQLOLEDB; Data Source=" & dbhost & "; Initial Catalog=" & dbname & "; User ID=" & dbuser & "; Password=" & dbpass

Set RS = Server.CreateObject("ADODB.Recordset")
Set RS_VER = Server.CreateObject("ADODB.Recordset")

Dim strReturn

QUERY_VER = "SELECT * FROM CT WHERE userid='" & userid & "'"
RS_VER.Open QUERY_VER, objConn, 3, 1

if RS_VER.RecordCount = 1 Then

	QUERY = "SELECT ClanZang FROM CL WHERE ClanName='" & clName & "'"
	RS.Open QUERY, objConn, 3, 1
	
	if Not RS.RecordCount >= 1 Then
	strReturn = "Code=0" & strSplit
	
	RS.Close
	Set RS = Nothing
	objConn.Close
	Set objConn = Nothing
	
	Response.Write(strReturn)
	Response.End
	End if
	
	Dim ClanLeader
	ClanLeader = RS("ClanZang").Value
	
	if Not chName = ClanLeader Then
	strReturn = "Code=0" & strSplit
	
	RS.Close
	Set RS = Nothing
	objConn.Close
	Set objConn = Nothing
	
	Response.Write(strReturn)
	Response.End
	End if
	
	RS.Close
	
	QUERY = "DELETE FROM CL WHERE ClanName='" & clName & "'"
	RS.Open QUERY, objConn, 3, 1
	
	strReturn = "Code=1" & strSplit
	
	QUERY = "DELETE FROM UL WHERE ClanName='" & clName & "'"
	RS.Open QUERY, objConn, 3, 1
	
	Set RS = Nothing
	objConn.Close
	Set objConn = Nothing
	
	Response.Write(strReturn)
	
else

	Response.Write("voc&ecirc; est&aacute; tentando deletar o clan pelo navegador.")
	
end if

	Set RS_VER  = Nothing
%>
