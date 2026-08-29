<%@ Page Language="VB" %>
<%@ import Namespace="System.Data" %>
<%@ import Namespace="System.Data.SqlClient" %>
<script runat="server">

Function GetParameter(pName as string, pType as string) as string
dim lTmp as string
lTmp = Page.Request.QueryString.item(pName)

GetParameter = lTmp
End Function

Sub Page_Load(Sender As Object, E As EventArgs)

Dim dbhost, dbuser, dbpass, dbname, userid, gserver, chname, clName, expl, chtype, lv

dbhost = "PTL\SQLEXPRESS"
dbuser = "sa"
dbpass = "#$pdl32xq"
dbname = "ClanDB"

Dim strSplit As String
strSplit = Chr("&H" & "0D")

userid = GetParameter ("userid","String")
gserver = GetParameter ("gserver","String")
chname = GetParameter("chname","String")
clName = GetParameter("clName","String")
chtype = GetParameter("chtype","String")
lv = GetParameter("lv","String")

expl = "PT"

if userid="" Or gserver="" Or chname="" Or clName="" Or chtype="" or lv="" Then
Response.Write("Code=100" & strSplit)
Response.End
End if

Dim objConn, objReader, objCommand, objQuery

Dim connectionString As String = "server='" & dbhost & "'; user id='" & dbuser & "'; password='" & dbpass & "'; database='" & dbname & "'"

objConn = New SqlConnection(connectionString)
objConn.Open()

objQuery = "SELECT ClanName FROM UL WHERE ChName='" & chname & "'"

objCommand = New SqlCommand(objQuery, objConn)
objReader = objCommand.ExecuteReader()

If objReader.Read()
If objReader.Item(0) = "" Then
objReader.Close()
objQuery = "DELETE FROM UL WHERE ChName='" & chname & "'"

objCommand = New SqlCommand(objQuery, objConn)
objReader = objCommand.ExecuteReader()
Else
objReader.Close()
objConn.Close()
Response.Write("Code=2" & strSplit & "CMoney=0" & strSplit)
Response.End
End if
End if
objReader.Close()

objQuery = "SELECT ClanZang FROM CL WHERE ClanName='" & clName & "'"

objCommand = New SqlCommand(objQuery, objConn)
objReader = objCommand.ExecuteReader()

If objReader.Read()
objReader.Close()
objConn.Close()
Response.Write("Code=3" & strSplit & "CMoney=0" & strSplit)
Response.End
End if
objReader.Close()

objQuery = "SELECT IMG FROM LI WHERE ID=1"

objCommand = New SqlCommand(objQuery, objConn)
objReader = objCommand.ExecuteReader()

Dim iIMG As Integer

if objReader.Read() Then
iIMG = objReader.Item(0)
Else
iIMG = 1000000000
objReader.Close()
objQuery = "INSERT INTO LI values('" & (iIMG + 1) & "','1')"
objCommand = New SqlCommand(objQuery, objConn)
objReader = objCommand.ExecuteReader()
End if
objReader.Close()

iIMG = iIMG + 1

objQuery = "UPDATE LI SET IMG='" & iIMG & "' WHERE ID=1"

objCommand = New SqlCommand(objQuery, objConn)
objReader = objCommand.ExecuteReader()
objReader.Close()

objQuery = "INSERT INTO CL ([ClanName],[UserID],[ClanZang],[MemCnt],[Note],[MIconCnt],[RegiDate],[LimitDate],[DelActive],[PFlag],[KFlag],[Flag],[NoteCnt],[Cpoint],[CWin],[CFail],[ClanMoney],[CNFlag],[SiegeMoney]) values('" & clName & "','" & userid & "','" & chname & "','1','" & expl & "','" & iIMG & "',getdate(),getdate()+3600,'0','0','0','0','1 ','0 ','0','0','0','0','0')"
objCommand = New SqlCommand(objQuery, objConn)
objReader = objCommand.ExecuteReader()
objReader.Close()

objQuery = "SELECT IDX FROM CL WHERE ClanName='" & clName & "'"
objCommand = New SqlCommand(objQuery, objConn)

Dim IDX As String
objReader = objCommand.ExecuteReader()
If objReader.Read() Then
IDX = objReader.Item(0)
End If
objReader.Close()

objQuery = "INSERT INTO UL ([IDX],[userid],[ChName],[ClanName],[ChType],[ChLv],[Permi],[JoinDate],[DelActive],[PFlag],[KFlag],[MIconCnt]) values('" & IDX & "','" & userid & "','" & chname & "','" & clName & "','" & chtype & "','" & lv & "','0',getdate(),'0','0','0','" & iIMG & "')"
objCommand = New SqlCommand(objQuery, objConn)
objReader = objCommand.ExecuteReader()
objReader.Close()

objConn.Close()

Response.Write("Code=1" & strSplit & "CMoney=500000" & strSplit)
End Sub

</script>
<html>
<head>
</head>
<body>
</body>
</html>