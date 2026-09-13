#include <windows.h>
#include <string.h>
#include <stdio.h>

#include "systems/social/chat/filter/CurseFilter.h"

#define MAX_CURSE_STRING	32768
#define HUGE_NUMBER		MAX_CURSE_STRING

static char curse_string[MAX_CURSE_STRING];
static bool bInit = false;
static char curse_stringID[MAX_CURSE_STRING];
static bool bInitID = false;

static int chksum;
static int chksumID;


static void filterChar(const char *src, char *dst, size_t dstSize, const char *chars)
{
	if(!dst || dstSize == 0) return;
	dst[0] = '\0';
	if(!src || !chars) return;

	const unsigned char *srcp = (const unsigned char *)src;
	char *dstp = dst;
	char *dstEnd = dst + dstSize - 1;

	while(*srcp && dstp < dstEnd)
	{
		if(strchr(chars, (char)*srcp) == NULL)
		{
			*dstp++ = (char)*srcp;
		}
		else if(srcp > (const unsigned char *)src)
		{
			const int msb = *(srcp - 1);
			const int lsb = *srcp;
			if((msb == 0x82 && lsb > 0x9e && lsb < 0xf2)
				|| (msb == 0x83 && lsb > 0x3f && lsb < 0x97)
				|| (msb >= 0x88 && msb <= 0x9f && msb != 0x7f
					&& lsb > 0x3f && lsb < 0xfd)
				|| (msb >= 0xe0 && msb <= 0xfc && msb != 0x7f
					&& lsb > 0x3f && lsb < 0xfd))
			{
				*dstp++ = (char)*srcp;
			}
		}
		++srcp;
	}
	*dstp = '\0';
}

static char *getField(const char *buf, int delim, char *field, size_t fieldSize)
{
	if(!field || fieldSize == 0) return (char *)buf;
	field[0] = '\0';
	if(!buf) return NULL;

	const unsigned char *src = (const unsigned char *)buf;
	char *dst = field;
	char *dstEnd = field + fieldSize - 1;

	while((int)*src == delim && *src != 0) ++src;
	while((int)*src != delim && *src != 0)
	{
		if(dst < dstEnd) *dst++ = (char)*src;
		++src;
	}
	*dst = '\0';
	return (char *)src;
}

static size_t appendBuffer(char *buffer, size_t capacity, size_t used, const char *data, size_t dataLen)
{
	if(!buffer || capacity == 0) return 0;
	if(!data || used >= capacity - 1)
	{
		buffer[capacity - 1] = '\0';
		return used < capacity ? used : capacity - 1;
	}

	const size_t available = capacity - 1 - used;
	const size_t copyLen = dataLen < available ? dataLen : available;
	if(copyLen) memcpy(buffer + used, data, copyLen);
	used += copyLen;
	buffer[used] = '\0';
	return used;
}

static BOOL replaceAllBounded(char *buffer, size_t bufferSize, size_t maxLen, const char *needle, const char *replacement)
{
	if(!buffer || bufferSize == 0 || !needle || !needle[0] || !replacement) return FALSE;
	if(maxLen >= bufferSize) maxLen = bufferSize - 1;

	const size_t needleLen = strlen(needle);
	const size_t replacementLen = strlen(replacement);
	char result[MAX_CURSE_STRING] = {};
	const char *read = buffer;
	size_t used = 0;
	BOOL changed = FALSE;

	while(*read && used < maxLen)
	{
		const char *hit = strstr(read, needle);
		if(!hit)
		{
			size_t tail = strlen(read);
			if(tail > maxLen - used) tail = maxLen - used;
			if(tail) memcpy(result + used, read, tail);
			used += tail;
			break;
		}

		size_t prefix = (size_t)(hit - read);
		if(prefix > maxLen - used) prefix = maxLen - used;
		if(prefix) memcpy(result + used, read, prefix);
		used += prefix;
		if(used >= maxLen) break;

		const char *after = hit + needleLen;
		const size_t tailLen = strlen(after);
		size_t insertLen = replacementLen;
		// Preserve legacy behavior: if replacement would exceed max_len,
		// remove the matched curse instead of overflowing the destination.
		if(used + insertLen + tailLen > maxLen) insertLen = 0;
		if(insertLen > maxLen - used) insertLen = maxLen - used;
		if(insertLen) memcpy(result + used, replacement, insertLen);
		used += insertLen;
		read = after;
		changed = TRUE;
	}

	result[used] = '\0';
	memcpy(buffer, result, used + 1);
	return changed;
}

int LoadCurses(const char *filename)
{

	static char comment_chars[] = "\n";
	char buf[128];
	size_t used = 0;
	FILE *file;

	char passcode[128];

	if(bInit) return FALSE;


	if(file = fopen(filename, "r"), !file)
	{
		fprintf(stderr, "! can't open [%s]\n", filename);
		return FALSE;
	}

	while(fgets(buf, sizeof(buf), file))
	{
		if(strchr(comment_chars, buf[0])) continue;

		ZeroMemory(passcode, 128);
		PassCode(buf, passcode, strlen(buf));
		const size_t encodedLen = strlen(passcode);
		const size_t before = used;
		used = appendBuffer(curse_string, sizeof(curse_string), used, passcode, encodedLen);



		if(used == sizeof(curse_string) - 1 && before + encodedLen > used)
		{
			fprintf(stderr, "! %s is too long, pass.\n", filename);
			curse_string[MAX_CURSE_STRING - 1] = 0;
			break;
		}
	}
	fclose(file);

	fprintf(stderr, "* parsing [%s]. %d bytes read\n", filename, (int)used);
	bInit = true;
	return chksum;
}

int LoadCursesID(const char *filename)
{

	static char comment_chars[] = "\n";
	char buf[128];
	size_t used = 0;
	FILE *file;

	char passcode[128];

	if(bInitID) return FALSE;


	if(file = fopen(filename, "r"), !file)
	{
		fprintf(stderr, "! can't open [%s]\n", filename);
		return FALSE;
	}

	while(fgets(buf, sizeof(buf), file))
	{
		if(strchr(comment_chars, buf[0])) continue;

		ZeroMemory(passcode, 128);
		PassCodeID(buf, passcode, strlen(buf));
		const size_t encodedLen = strlen(passcode);
		const size_t before = used;
		used = appendBuffer(curse_stringID, sizeof(curse_stringID), used, passcode, encodedLen);



		if(used == sizeof(curse_stringID) - 1 && before + encodedLen > used)
		{
			fprintf(stderr, "! %s is too long, pass.\n", filename);
			curse_stringID[MAX_CURSE_STRING - 1] = 0;
			break;
		}
	}
	fclose(file);

	fprintf(stderr, "* parsing [%s]. %d bytes read\n", filename, (int)used);
	bInitID = true;
	return chksumID;
}


int LoadCursesDebug(const char *filename)
{

	static char comment_chars[] = "\n";
	char buf[128];
	size_t used = 0;
	FILE *file;

	if(bInit) return FALSE;


	if(file = fopen(filename, "r"), !file)
	{
		fprintf(stderr, "! can't open [%s]\n", filename);
		return FALSE;
	}

	while(fgets(buf, sizeof(buf), file))
	{
		if(strchr(comment_chars, buf[0])) continue;

		const size_t lineLen = strlen(buf);
		const size_t before = used;
		used = appendBuffer(curse_string, sizeof(curse_string), used, buf, lineLen);

		if(used == sizeof(curse_string) - 1 && before + lineLen > used)
		{
			fprintf(stderr, "! %s is too long, pass.\n", filename);
			curse_string[MAX_CURSE_STRING - 1] = 0;
			break;
		}
	}
	fclose(file);

	fprintf(stderr, "* parsing [%s]. %d bytes read\n", filename, (int)used);
	bInit = true;
	return chksum;
}

int LoadCursesDebugID(const char *filename)
{

	static char comment_chars[] = "\n";
	char buf[128];
	size_t used = 0;
	FILE *file;

	if(bInitID) return FALSE;


	if(file = fopen(filename, "r"), !file)
	{
		fprintf(stderr, "! can't open [%s]\n", filename);
		return FALSE;
	}

	while(fgets(buf, sizeof(buf), file))
	{
		if(strchr(comment_chars, buf[0])) continue;

		const size_t lineLen = strlen(buf);
		const size_t before = used;
		used = appendBuffer(curse_stringID, sizeof(curse_stringID), used, buf, lineLen);

		if(used == sizeof(curse_stringID) - 1 && before + lineLen > used)
		{
			fprintf(stderr, "! %s is too long, pass.\n", filename);
			curse_stringID[MAX_CURSE_STRING - 1] = 0;
			break;
		}
	}
	fclose(file);

	fprintf(stderr, "* parsing [%s]. %d bytes read\n", filename, (int)used);
	bInitID = true;
	return chksumID;
}

bool IsCurse(const char * str)
{
	char string[HUGE_NUMBER], line[HUGE_NUMBER], word[HUGE_NUMBER];
	char *ptr = curse_string;
	const char *sub, *subs, *subw;

	if(!str) return false;





	filterChar(str, string, sizeof(string), " ");




	while(ptr = getField(ptr, '\n', line, sizeof(line)), line[0])
	{
		getField(line, '\t', word, sizeof(word));
		sub = string;

		while(*sub)
		{
			subs = sub;
			subw = word;
			while(*subs && (*subs == *subw))
			{
				subs++; subw++;
				if(*subw == 0)
				{
					return true;
				}
			}
			sub += *sub < 0 ? 2 : 1;
		}
	}

	return false;
}

bool IsCurseID(const char * str)
{
	char string[HUGE_NUMBER], line[HUGE_NUMBER], word[HUGE_NUMBER];
	char *ptr = curse_stringID;
	const char *sub, *subs, *subw;

	if(!str) return false;





	filterChar(str, string, sizeof(string), " ");




	while(ptr = getField(ptr, '\n', line, sizeof(line)), line[0])
	{
		getField(line, '\t', word, sizeof(word));
		sub = string;

		while(*sub)
		{
			subs = sub;
			subw = word;
			while(*subs && (*subs == *subw))
			{
				subs++; subw++;
				if(*subw == 0)
				{
					return true;
				}
			}
			sub += *sub < 0 ? 2 : 1;
		}
	}
	return false;
}

char *ConvertString(char * str, int max_len)
{
	char string[MAX_CURSE_STRING];
	char line[128], curse[128], replace[128];
	char *ptr = curse_string, *lptr;

	if(!str) return NULL;
	if(max_len <= 0 || max_len >= (int)sizeof(string)) return str;
	if(!IsCurse(str)) return str;

	filterChar(str, string, sizeof(string), " ");
	string[max_len] = '\0';

	while(ptr = getField(ptr, '\n', line, sizeof(line)), line[0])
	{
		lptr = line;
		lptr = getField(lptr, '\t', curse, sizeof(curse));
		lptr = getField(lptr, '\t', replace, sizeof(replace));
		if(!curse[0]) continue;
		if(!replace[0] && strstr(string, curse)) return NULL;
		replaceAllBounded(string, sizeof(string), (size_t)max_len, curse, replace);
	}

	memcpy(str, string, strlen(string) + 1);
	return str;
}

char *ConvertStringTHAI(char * str, int max_len)
{
	char string[MAX_CURSE_STRING];
	char line[128], curse[128], replace[128];
	char *ptr = curse_string, *lptr;

	if(!str) return NULL;
	if(max_len <= 0 || max_len >= (int)sizeof(string)) return str;

	filterChar(str, string, sizeof(string), "");
	string[max_len] = '\0';

	while(ptr = getField(ptr, '\n', line, sizeof(line)), line[0])
	{
		lptr = line;
		lptr = getField(lptr, '\t', curse, sizeof(curse));
		if(!curse[0]) continue;

		size_t maskLen = strlen(curse);
		if(maskLen >= sizeof(replace)) maskLen = sizeof(replace) - 1;
		memset(replace, '*', maskLen);
		replace[maskLen] = '\0';
		replaceAllBounded(string, sizeof(string), (size_t)max_len, curse, replace);
	}

	memcpy(str, string, strlen(string) + 1);
	return str;
}

char *ConvertStringTaiwanChat(char * str, int max_len)
{
	return ConvertStringTHAI(str, max_len);
}

bool BadTaiwanID(const char * str)
{
	return IsCurseID(str);
}


void PassCode(char PassBefore[], char PassAfter[], int number)

{
	int j;

	for(j = 0; j < number; j++)
	{
		chksum += PassBefore[j];
		PassAfter[j] = PassBefore[j] ^ 0x1;
	}
}

void PassCodeID(char PassBefore[], char PassAfter[], int number)

{
	int j;

	for(j = 0; j < number; j++)
	{
		chksumID += PassBefore[j];
		PassAfter[j] = PassBefore[j] ^ 0x1;
	}
}

int ChatChkSum()
{
	return chksum;
}

int ChatChkSumID()
{
	return chksumID;
}




