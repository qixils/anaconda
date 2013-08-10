#include	"common.h"

bool AssertFail(const wchar_t* expression, const wchar_t* file, int line)
{
	char buffer[256];
	sprintf(buffer,"Assertion Failed at: %ws\n\nFile %ws\nLine %d\n\nPress OK to continue or Cancel to Halt the Program.",expression,file,line);
	std::cout << buffer << std::endl;
	return true;
}

bool AssertFail2(const char* message)
{
	char buffer[256];
	sprintf(buffer,"%s\n\nPress OK to continue or Cancel to Halt the Program.",message);
	std::cout << buffer << std::endl;
	return true;
}

void Debug(const char* str, ...)
{
	char buffer[256];
	if(!str) return;

	va_list ptr;
	va_start(ptr,str);
	vsnprintf(buffer,256,str,ptr);
	va_end(ptr);

	std::cout << buffer << std::endl;
}

void Debug(int d)
{
	Debug("%d",d);
}

void Debug(float d)
{
	Debug("%f",d);
}


// ----------------
// HandleRunObject
// ----------------
// Called (if you want) each loop, this routine makes the object live
// 
short WINAPI DLLExport HandleRunObject(LPRDATA rdPtr)
{
	// Will not be called next loop	
	//return REFLAG_ONESHOT;
	if(rdPtr->autoUpdate)
	{
		ActionFunc0(rdPtr, 0, 0);
	}
	else if(rdPtr->callbacks)
	{
		while(rdPtr->callbacks)
		{
			rdPtr->callbacks->Do(rdPtr);
			Callback* c = rdPtr->callbacks->Next;
			delete rdPtr->callbacks;
			rdPtr->callbacks = c;
		}
		rdPtr->lastcall = NULL;
	}
    return 0;
}