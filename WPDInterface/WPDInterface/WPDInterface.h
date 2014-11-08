// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the WPDINTERFACE_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// WPDINTERFACE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#include <Windows.h>

#define MAX_REQUESTED_ELEMENTS 20

extern "C" {

	typedef struct WPDFileInfo {
		WCHAR * id;
		WCHAR * name;
		bool isDir;
		WCHAR * date;
		UINT64 size;
	} WPDFileInfo;

	__declspec(dllexport) void WPDI_Init(void);
	__declspec(dllexport) void WPDI_Free(void * p);
	__declspec(dllexport) void WPDI_LookForNewDevice(WCHAR ** id, WCHAR ** displayName, WCHAR ** manufacturer, WCHAR ** description);
	__declspec(dllexport) void WPDI_LS(PWSTR deviceID, PWSTR objectID, WPDFileInfo* result, int * count);

	__declspec(dllexport) bool WPDI_InitTransfer(PWSTR deviceID, PWSTR objectID, DWORD *optimalTransferSize);
	__declspec(dllexport) bool WPDI_readNextData(BYTE * data, DWORD size, DWORD *read);
	__declspec(dllexport) void WPDI_CloseTransfer(void);
}
