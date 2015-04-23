#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "StrPtrLen.h"
#include "ConfigPrefsParser.h"


ConfigPrefsParser::ConfigPrefsParser(const char *theFilePath)
 :	fFile(-1)
{
    strncpy(fFilePath, theFilePath, kFilePathSize);
}

bool ConfigPrefsParser::DoesFileExist()
{
    bool itExists = false;
	UInt64 theLength = 0;
	bool isDir = true;

	fFile = open(fFilePath, O_RDONLY | O_LARGEFILE);

	if (fFile != -1)
    {
    	struct stat buf;
        memset(&buf,sizeof(buf),0);
    	if (fstat(fFile, &buf) != -1)
        {
        	theLength = buf.st_size;

        	isDir = S_ISDIR(buf.st_mode);
        }
    	else
            close(fFile);
    }

    if ((theLength > 0) && (!isDir))
        itExists = true;

    return itExists;
}

void ConfigPrefsParser::SetValue(char* theLine, char* theName, char* theType, UInt32 theIndex)
{
	strncpy(fConfigParameterList[theIndex].fName, theName, ConfigParameter::kParameterNameSize);
	if (strcmp(theType, "uint32") == 0)
	{
		sscanf(theLine, "%*[^=]=%u", (UInt32*)fConfigParameterList[theIndex].fValue);
		//#if CONFIG_PREFS_PARSER_DEBUG
		//	printf("%s.....%s.....%u\n", fConfigParameterList[theIndex].fName, theType, *(UInt32*)fConfigParameterList[theIndex].fValue);
		//#endif
	}
	else if (strcmp(theType, "int32") == 0)
	{
		sscanf(theLine, "%*[^=]=%d", (UInt32*)fConfigParameterList[theIndex].fValue);
		//#if CONFIG_PREFS_PARSER_DEBUG
		//	printf("%s.....%s.....%d\n", fConfigParameterList[theIndex].fName, theType, *(UInt32*)fConfigParameterList[theIndex].fValue);
		//#endif
	}
	else if (strcmp(theType, "uint16") == 0)
	{
		sscanf(theLine, "%*[^=]=%hu", (UInt16*)fConfigParameterList[theIndex].fValue);
		//#if CONFIG_PREFS_PARSER_DEBUG
		//	printf("%s.....%s.....%hu\n", fConfigParameterList[theIndex].fName, theType, *(UInt16*)fConfigParameterList[theIndex].fValue);
		//#endif
	}
	else if (strcmp(theType, "string") == 0)
	{
		sscanf(theLine, "%*[^=]=%s", fConfigParameterList[theIndex].fValue);
		//#if CONFIG_PREFS_PARSER_DEBUG
		//	printf("%s.....%s.....%s\n", fConfigParameterList[theIndex].fName, theType, fConfigParameterList[theIndex].fValue);
		//#endif
	}
}

void* ConfigPrefsParser::GetValue(const char* theName)
{
	UInt32 i = 0;
	for (i = 0; i < fTotalParameterNum; i++)
	{
		if (strcmp(fConfigParameterList[i].fName, theName) != 0)
			continue;

		return (void *)fConfigParameterList[i].fValue;
	}

	return NULL;
}

int ConfigPrefsParser::Parse()
{
	FILE *theConfigFile = NULL;
	char *theLine = NULL;
	char theData[1024];
	char theParameterName[64];
	char theType[8];
	UInt32 theIndex = 0;

	theConfigFile = ::fopen(fFilePath, "r");
	if(theConfigFile == NULL) return -1;

	while((theLine = fgets(theData, 1024, theConfigFile)) != NULL)
	{
		sscanf(theLine, "%[^:]::%[^=]", theParameterName, theType);
		this->SetValue(theLine, theParameterName, theType, theIndex);
		theIndex++;
	}

	fTotalParameterNum = theIndex;
	
    fclose(theConfigFile);
    
	return 0;    
}

