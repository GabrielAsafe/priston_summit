

#include "compat/legacy/sin/sinLinkHeader.h"



sinSTRING::sinSTRING(void)
{
	m_aString = new char[1];
	m_aString[0] = '\0';
}

sinSTRING::sinSTRING(sinSTRING &rString)
{
	const int length = rString.GetLength();
	m_aString = new char[length + 1];
	if(length > 0) memcpy(m_aString, rString.GetString(), length);
	m_aString[length] = '\0';
}

sinSTRING::sinSTRING(char *pString)
{
	const size_t length = pString ? strlen(pString) : 0;
	m_aString = new char[length + 1];
	if(length > 0) memcpy(m_aString, pString, length);
	m_aString[length] = '\0';
}

sinSTRING::~sinSTRING(void)
{
	delete[] m_aString;
	m_aString = NULL;
}

sinSTRING & sinSTRING::operator = (sinSTRING &rString)
{
	if(this == &rString) return *this;
	return operator = (rString.GetString());
}

sinSTRING & sinSTRING::operator = (char *pString)
{
	const size_t length = pString ? strlen(pString) : 0;
	char *newString = new char[length + 1];
	if(length > 0) memcpy(newString, pString, length);
	newString[length] = '\0';
	delete[] m_aString;
	m_aString = newString;
	return *this;
}

bool sinSTRING::operator == (sinSTRING &rString)
{
	return strcmp(rString.GetString(), m_aString) == 0;
}

bool sinSTRING::operator == (char *pString)
{
	if(!pString) return m_aString[0] == '\0';
	return strcmp(pString, m_aString) == 0;
}

bool sinSTRING::operator != (sinSTRING &rString)
{
	return !(*this == rString);
}

bool sinSTRING::operator != (char *pString)
{
	return !(*this == pString);
}

char *sinSTRING::GetString(void)
{
	return m_aString;
}

int sinSTRING::GetLength(void)
{
	return (int)strlen(m_aString);
}


int halpRelease(LPDIRECT3DTEXTURE9 lpTemp)
{
	if(!lpTemp) return FALSE;

	if(lpTemp)
	{
		lpTemp->Release();
		lpTemp = 0;
	}
	return TRUE;
}

int haSearchString(char *StrBuffer, char KeyBuffer)
{
	int  TotalNum = 0;

	TotalNum = lstrlen(StrBuffer);

	for(int i = 0; i < TotalNum; i++)
	{
		if(StrBuffer[i] == KeyBuffer)
		{
			return i + 1;
		}
	}
	return 0;
}

int haStringCenter(int PosiX, char *strbuff)
{

	int len = 0;
	int rPosiX = 0;
	len = lstrlen(strbuff);


	if(len % 2)
		rPosiX += 4;

	if(PosiX % 2)
		rPosiX += 1;


	rPosiX = PosiX / 2 - (len / 2 * 8);


	return rPosiX;
}




