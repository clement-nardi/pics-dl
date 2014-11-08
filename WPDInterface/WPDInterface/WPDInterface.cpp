// WPDInterface.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "WPDInterface.h"
#include <iostream>

static IPortableDeviceManager *WPDManager;

static char * getDeviceID();
static PWSTR* previousDevices;
static DWORD  previousNbDevices;
static IPortableDevice* pDevice;
static IPortableDeviceValues *clientInfo;

/* used by WPDI_LS */
static PWSTR lastDeviceID;
static PWSTR lastObjectID;
static IPortableDeviceContent *pContent;
static IPortableDeviceKeyCollection *pPropertiesToRead;
static IEnumPortableDeviceObjectIDs *pEnumObjectIDs;
static IPortableDeviceProperties    *pProperties;

static void GetClientInformation(
	IPortableDeviceValues** ppClientInformation)
{
	HRESULT hr = CoCreateInstance(CLSID_PortableDeviceValues,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(ppClientInformation));
	if (SUCCEEDED(hr)) {
		hr = (*ppClientInformation)->SetStringValue(WPD_CLIENT_NAME, L"WPD Interface DLL");
		if (FAILED(hr)) { printf("! Failed to set WPD_CLIENT_NAME, hr = 0x%lx\n", hr); }
		hr = (*ppClientInformation)->SetUnsignedIntegerValue(WPD_CLIENT_MAJOR_VERSION, 1);
		if (FAILED(hr))	{ printf("! Failed to set WPD_CLIENT_MAJOR_VERSION, hr = 0x%lx\n", hr);	}
		hr = (*ppClientInformation)->SetUnsignedIntegerValue(WPD_CLIENT_MINOR_VERSION, 0);
		if (FAILED(hr)) { printf("! Failed to set WPD_CLIENT_MINOR_VERSION, hr = 0x%lx\n", hr); }
		hr = (*ppClientInformation)->SetUnsignedIntegerValue(WPD_CLIENT_REVISION, 1);
		if (FAILED(hr))	{ printf("! Failed to set WPD_CLIENT_REVISION, hr = 0x%lx\n", hr); }
		hr = (*ppClientInformation)->SetUnsignedIntegerValue(WPD_CLIENT_SECURITY_QUALITY_OF_SERVICE, SECURITY_IMPERSONATION);
		if (FAILED(hr))	{ printf("! Failed to set WPD_CLIENT_SECURITY_QUALITY_OF_SERVICE, hr = 0x%lx\n", hr); }
	} else { 
		printf("! Failed to CoCreateInstance CLSID_PortableDeviceValues, hr = 0x%lx\n", hr);	
	}
}

__declspec(dllexport) void WPDI_Free(void * p) {
	free(p);
}

__declspec(dllexport) void WPDI_Init(void) {
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	previousDevices = NULL;
	previousNbDevices = 0;
	lastDeviceID = (PWSTR) malloc(sizeof(WCHAR)* 2);
	lastObjectID = (PWSTR) malloc(sizeof(WCHAR)* 2);
	lastDeviceID[0] = L'\0';
	lastObjectID[0] = L'\0';
	if (SUCCEEDED(hr) || hr == RPC_E_CHANGED_MODE) {
		if (hr == RPC_E_CHANGED_MODE) {
			std::cout << "WARNING: CoInitializeEx returned RPC_E_CHANGED_MODE\n";
		}
		else {
			std::cout << "CoInitialize Succeeded\n";
		}
		hr = CoCreateInstance(CLSID_PortableDeviceManager,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&WPDManager));
		if (SUCCEEDED(hr)) {
			std::cout << "CoCreateInstance of WPDManager Succeeded\n";
			WCHAR * a;
			WCHAR * b;
			WCHAR * c;
			WCHAR * d;
			WPDI_LookForNewDevice(&a,&b,&c,&d);
		}
		else {
			WPDManager = NULL;
			std::cout << "CoCreateInstance of WPDManager failed\n";
		}

		hr = CoCreateInstance(CLSID_PortableDeviceFTM,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&pDevice));
		if (SUCCEEDED(hr)) {
			std::cout << "CoCreateInstance of pDevice Succeeded\n";
		}
		else {
			pDevice = NULL;
			std::cout << "CoCreateInstance of pDevice failed\n";
		}

		GetClientInformation(&clientInfo);
	} else {
		printf("CoInitialize Failed hr = 0x%lx\n",hr);
	}
	
	hr = CoCreateInstance(CLSID_PortableDeviceKeyCollection,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&pPropertiesToRead));
	if (!SUCCEEDED(hr)) { printf("CoCreateInstance pPropertiesToRead failed, hr = 0x%lx\n", hr); std::cout.flush(); }
	hr = pPropertiesToRead->Add(WPD_OBJECT_PARENT_ID);
	if (FAILED(hr)) { printf("! Failed to add WPD_OBJECT_PARENT_ID to IPortableDeviceKeyCollection, hr = 0x%lx\n", hr); std::cout.flush(); }
	hr = pPropertiesToRead->Add(WPD_OBJECT_NAME);
	if (FAILED(hr)) { printf("! Failed to add WPD_OBJECT_NAME to IPortableDeviceKeyCollection, hr = 0x%lx\n", hr); std::cout.flush(); }
	hr = pPropertiesToRead->Add(WPD_OBJECT_PERSISTENT_UNIQUE_ID);
	if (FAILED(hr)) { printf("! Failed to add WPD_OBJECT_PERSISTENT_UNIQUE_ID to IPortableDeviceKeyCollection, hr = 0x%lx\n", hr); std::cout.flush(); }
	hr = pPropertiesToRead->Add(WPD_OBJECT_FORMAT);
	if (FAILED(hr)) { printf("! Failed to add WPD_OBJECT_FORMAT to IPortableDeviceKeyCollection, hr = 0x%lx\n", hr); std::cout.flush(); }
	hr = pPropertiesToRead->Add(WPD_OBJECT_CONTENT_TYPE);
	if (FAILED(hr)) { printf("! Failed to add WPD_OBJECT_CONTENT_TYPE to IPortableDeviceKeyCollection, hr = 0x%lx\n", hr); std::cout.flush(); }
	hr = pPropertiesToRead->Add(WPD_OBJECT_ORIGINAL_FILE_NAME);
	if (FAILED(hr)) { printf("! Failed to add WPD_OBJECT_ORIGINAL_FILE_NAME to IPortableDeviceKeyCollection, hr = 0x%lx\n", hr); std::cout.flush(); }
	hr = pPropertiesToRead->Add(WPD_OBJECT_DATE_MODIFIED);
	if (FAILED(hr)) { printf("! Failed to add WPD_OBJECT_DATE_MODIFIED to IPortableDeviceKeyCollection, hr = 0x%lx\n", hr); std::cout.flush(); }
	hr = pPropertiesToRead->Add(WPD_OBJECT_SIZE);
	if (FAILED(hr)) { printf("! Failed to add WPD_OBJECT_DATE_MODIFIED to IPortableDeviceKeyCollection, hr = 0x%lx\n", hr); std::cout.flush(); }
	
	std::cout.flush();
}

static WCHAR* getManufacturer(PWSTR device) {
	DWORD   size = 0;
	PWSTR   manufacturer = NULL;
	HRESULT hr;
	if (WPDManager == NULL) { std::cout << "WPDManager is NULL\n"; std::cout.flush(); return NULL; }
	hr = WPDManager->GetDeviceManufacturer(device, NULL, &size);
	if (!SUCCEEDED(hr)) { std::cout << "WPDManager->GetDeviceManufacturer(NULL) failed\n"; std::cout.flush(); return NULL; }
	manufacturer = new (std::nothrow) WCHAR[size];
	hr = WPDManager->GetDeviceManufacturer(device, manufacturer, &size);
	if (!SUCCEEDED(hr)) { std::cout << "WPDManager->GetDeviceManufacturer(manufacturer) failed\n"; std::cout.flush(); return NULL; }
	return manufacturer;
}

static WCHAR* getDescription(PWSTR device) {
	DWORD   size = 0;
	PWSTR   description = NULL;
	HRESULT hr;
	if (WPDManager == NULL) { std::cout << "WPDManager is NULL\n"; std::cout.flush(); return NULL; }
	hr = WPDManager->GetDeviceDescription(device, NULL, &size);
	if (!SUCCEEDED(hr)) { std::cout << "WPDManager->GetDeviceDescription(NULL) failed\n"; std::cout.flush(); return NULL; }
	description = new (std::nothrow) WCHAR[size];
	hr = WPDManager->GetDeviceDescription(device, description, &size);
	if (!SUCCEEDED(hr)) { std::cout << "WPDManager->GetDeviceDescription(description) failed\n"; std::cout.flush(); return NULL; }
	return description;
}

static WCHAR* getFriendlyName(PWSTR device) {
	DWORD   size = 0;
	PWSTR   friendlyName = NULL;
	HRESULT hr;
	if (WPDManager == NULL) { std::cout << "WPDManager is NULL\n"; std::cout.flush(); return NULL; }
	hr = WPDManager->GetDeviceFriendlyName(device, NULL, &size);
	if (!SUCCEEDED(hr)) { std::cout << "WPDManager->GetDeviceFriendlyName(NULL) failed\n"; std::cout.flush(); return NULL; }
	friendlyName = new (std::nothrow) WCHAR[size];
	hr = WPDManager->GetDeviceFriendlyName(device, friendlyName, &size);
	if (!SUCCEEDED(hr)) { std::cout << "WPDManager->GetDeviceFriendlyName(friendlyName) failed\n"; std::cout.flush(); return NULL; }
	return friendlyName;
}

static bool isStandardDrive(PWSTR device) {
	WCHAR* fn = getFriendlyName(device);
	if (fn == NULL) return false;
	return fn[1] == L':' && fn[2] == L'\\';
}

void RecursiveEnumerate(
	PCWSTR                  pszObjectID,
	IPortableDeviceContent* pContent);
void ReadHintLocations(
	IPortableDevice* pDevice);
void ReadContentPropertiesBulk(
	IPortableDevice*    pDevice);


// Displays a property assumed to be in string form.
void DisplayStringProperty(
	IPortableDeviceValues*  pProperties,
	REFPROPERTYKEY          key,
	PCWSTR                  pszKey);

// Displays a property assumed to be in GUID form.
void DisplayGuidProperty(
	IPortableDeviceValues*  pProperties,
	REFPROPERTYKEY          key,
	PCWSTR                  pszKey);

static WCHAR* dupeStr(WCHAR* in){
	int size = wcslen(in);
	WCHAR * out = (PWSTR)malloc(sizeof(WCHAR)* (size + 1));
	wcsncpy_s(out, size + 1, in, size);
	out[size] = L'\0';
	return out;
}

bool openDevice(PWSTR deviceID, bool force = false) {
	if (wcscmp(lastDeviceID, deviceID) != 0 || force) {
		free(lastDeviceID);
		lastDeviceID = dupeStr(deviceID);
		HRESULT hr = pDevice->Open(deviceID, clientInfo);
		if (FAILED(hr))	{
			if (hr == E_ACCESSDENIED) {
				printf("Failed to Open the device for Read Write access, will open it for Read-only access instead\n"); std::cout.flush();
				clientInfo->SetUnsignedIntegerValue(WPD_CLIENT_DESIRED_ACCESS, GENERIC_READ);
				hr = pDevice->Open(deviceID, clientInfo);
				if (FAILED(hr))	{
					printf("! Failed to Open the device (RO), hr = 0x%lx\n", hr); std::cout.flush();
					return false;
				}
			}
			else {
				printf("! Failed to Open the device (RW), hr = 0x%lx\n", hr); std::cout.flush();
				return false;
			}
		}
		hr = pDevice->Content(&pContent);
		if (!SUCCEEDED(hr)) { std::cout << "pDevice->Content failed\n"; std::cout.flush(); return false; }
	}
	return true;
}


__declspec(dllexport) void WPDI_LS(PWSTR deviceID, PWSTR objectID, WPDFileInfo* res, int * count) {
	HRESULT hr;
	/*
	std::wcout << L"--> WPDI_LS(" << deviceID << L", " << objectID << L")";
	std::cout << "\n";
	std::cout.flush();*/

	*count = 0;

	/*
	std::wcout << L"lastDeviceID = " << lastDeviceID << L"\n"; std::wcout.flush();
	std::wcout << L"    deviceID = " << deviceID << L"\n"; std::wcout.flush();*/
	if (pDevice == NULL) { std::cout << "pDevice is NULL\n"; std::cout.flush(); return ; }
	if (!openDevice(deviceID)) {
		return;
	}

	CComPtr<IPortableDeviceValues>        pObjectProperties;
	
	/*
	std::wcout << L"lastObjectID = " << lastObjectID << L"\n"; std::wcout.flush();
	std::wcout << L"    objectID = " << objectID << L"\n"; std::wcout.flush();*/

	if (wcscmp(lastObjectID, objectID) != 0) {
		//printf("listing content of %ws\n", objectID);
		free(lastObjectID);
		lastObjectID = dupeStr(objectID);

		hr = pContent->EnumObjects(0,               // Flags are unused
			objectID,     // Starting from the passed in object
			NULL,            // Filter is unused
			&pEnumObjectIDs);
		if (hr == E_WPD_DEVICE_NOT_OPEN) {
			/* device was probably unplugged since last call */
			openDevice(deviceID, true);
			hr = pContent->EnumObjects(0,               // Flags are unused
				objectID,     // Starting from the passed in object
				NULL,            // Filter is unused
				&pEnumObjectIDs);
		}
		if (!SUCCEEDED(hr)) { printf("pContent->EnumObjects failed, hr = 0x%lx\n", hr); std::cout.flush(); return ; }
		hr = pContent->Properties(&pProperties);
		if (!SUCCEEDED(hr)) { printf("pContent->Properties failed, hr = 0x%lx\n", hr); std::cout.flush(); return ; }
	}
	else {
		//printf("continue listing content of %ws\n", objectID);
	}
	//std::cout.flush();

	DWORD  cFetched = 0;
	PWSTR  szObjectIDArray[MAX_REQUESTED_ELEMENTS] = { 0 };
	hr = pEnumObjectIDs->Next(MAX_REQUESTED_ELEMENTS,   // Number of objects to request on each NEXT call
		szObjectIDArray,          // Array of PWSTR array which will be populated on each NEXT call
		&cFetched);               // Number of objects written to the PWSTR array
	if (SUCCEEDED(hr)) {
		for (DWORD dwIndex = 0; dwIndex < cFetched; dwIndex++) {
			//RecursiveEnumerate(szObjectIDArray[dwIndex], pContent);
			//printf("%ws\n", szObjectIDArray[dwIndex]); std::cout.flush();
			res[*count].id = dupeStr(szObjectIDArray[dwIndex]);

			hr = pProperties->GetValues(szObjectIDArray[dwIndex],         // The object whose properties we are reading
				pPropertiesToRead,   // The properties we want to read
				&pObjectProperties); // Driver supplied property values for the specified object

			//DisplayStringProperty(pObjectProperties, WPD_OBJECT_PARENT_ID, L"WPD_OBJECT_PARENT_ID");
			//DisplayStringProperty(pObjectProperties, WPD_OBJECT_NAME, L"WPD_OBJECT_NAME");
			//DisplayStringProperty(pObjectProperties, WPD_OBJECT_ORIGINAL_FILE_NAME, L"WPD_OBJECT_ORIGINAL_FILE_NAME");
			PWSTR value = NULL;
			hr = pObjectProperties->GetStringValue(WPD_OBJECT_ORIGINAL_FILE_NAME, &value);
			if (SUCCEEDED(hr)) {
				res[*count].name = dupeStr(value);
			}
			else {
				CoTaskMemFree(value);
				value = NULL;
				hr = pObjectProperties->GetStringValue(WPD_OBJECT_NAME, &value);
				if (SUCCEEDED(hr)) {
					res[*count].name = dupeStr(value);
				}
				else {
					res[*count].name = NULL;
				}
			}
			CoTaskMemFree(value);
			value = NULL;
			hr = pObjectProperties->GetStringValue(WPD_OBJECT_DATE_MODIFIED, &value);
			if (SUCCEEDED(hr)) {
				res[*count].date = dupeStr(value);
			}
			else {
				res[*count].date = NULL;
			}
			CoTaskMemFree(value);
			value = NULL;
			pObjectProperties->GetUnsignedLargeIntegerValue(WPD_OBJECT_SIZE, &res[*count].size);
			//DisplayStringProperty(pObjectProperties, WPD_OBJECT_DATE_MODIFIED, L"WPD_OBJECT_DATE_MODIFIED");
			//DisplayStringProperty(pObjectProperties, WPD_OBJECT_PERSISTENT_UNIQUE_ID, L"WPD_OBJECT_PERSISTENT_UNIQUE_ID");
			//DisplayGuidProperty(pObjectProperties, WPD_OBJECT_CONTENT_TYPE, L"WPD_OBJECT_CONTENT_TYPE");
			GUID    guidValue = GUID_NULL;
			hr = pObjectProperties->GetGuidValue(WPD_OBJECT_CONTENT_TYPE, &guidValue);
			res[*count].isDir = (guidValue == WPD_CONTENT_TYPE_FOLDER || guidValue == WPD_CONTENT_TYPE_FUNCTIONAL_OBJECT);
			//DisplayGuidProperty(pObjectProperties, WPD_OBJECT_FORMAT, L"WPD_OBJECT_FORMAT");
			// Free allocated PWSTRs after the recursive enumeration call has completed.
			CoTaskMemFree(szObjectIDArray[dwIndex]);
			//szObjectIDArray[dwIndex] = NULL;
			(*count)++;
		}
	}
	std::cout.flush();
	return;
}
__declspec(dllexport) void WPDI_LookForNewDevice(WCHAR ** id, WCHAR ** displayName, WCHAR ** manufacturer, WCHAR ** description) {
	DWORD  nbDevices = 0;
	PWSTR* devices = NULL;
	HRESULT hr;
	*id = NULL;
	*displayName = NULL;
	*manufacturer = NULL;
	*description = NULL;

	std::cout << " --> WPDI_LookForNewDevice\n";
	std::cout.flush();
	if (WPDManager == NULL) { std::cout << "WPDManager is NULL\n"; std::cout.flush(); return ; }
	hr = WPDManager->RefreshDeviceList();
	std::cout << "    --> after WPDManager->RefreshDeviceList\n";
	std::cout.flush();
	hr = WPDManager->GetDevices(NULL, &nbDevices);
	if (!SUCCEEDED(hr)) { std::cout << "WPDManager->GetDevices(NULL) failed\n"; std::cout.flush(); return ; }
	devices = new (std::nothrow) PWSTR[nbDevices];
	if (devices == NULL) { std::cout << "devices is NULL"; std::cout.flush(); return ; }
	hr = WPDManager->GetDevices(devices, &nbDevices);
	if (!SUCCEEDED(hr)) { std::cout << "WPDManager->GetDevices(devices) failed\n"; std::cout.flush(); return ; }
	if (previousDevices != NULL) {
		std::cout << "Loop on devices\n"; std::cout.flush();
		for (int i = 0; i < nbDevices; i++) {
			//std::wcout << L"device[" << i << L"]= " << getFriendlyName(devices[i]) << L" " << getManufacturer(devices[i]) << L" " << getDescription(devices[i]) << L"\n";
			//std::wcout.flush();
			int j;
			for (j = 0; j < previousNbDevices; j++) {
				if (wcscmp(devices[i], previousDevices[j]) == 0) {
					break;
				}
			}
			if (j >= previousNbDevices) {
				/* This is a new device */
				std::cout << "Found a new device!! ";
				std::wcout << devices[i];
				std::cout << "\n";
				std::cout.flush();
				if (isStandardDrive(devices[i])) {
					std::cout << "  --> IGNORE this new device because it is a standard Drive\n"; std::cout.flush();
				} else {
					/* stop here, even if 2 devices were inserted at the same time */
					*id = dupeStr(devices[i]);
					*displayName = getFriendlyName(devices[i]);
					std::cout << "toto\n";
					std::cout.flush();
					*manufacturer = getManufacturer(devices[i]);
					std::cout << "titi\n";
					std::cout.flush();
					*description = getDescription(devices[i]);
					std::cout << "tata\n";
					std::cout.flush();
					return ;
				}
			}
		}
		delete previousDevices;
	}
	previousDevices = devices;
	previousNbDevices = nbDevices;
	return ;
}


static IPortableDeviceDataStream *pDataStream;
static IPortableDeviceResources  *pResources;

__declspec(dllexport) bool WPDI_InitTransfer(PWSTR deviceID, PWSTR objectID, DWORD *optimalTransferSize) {
	CComPtr<IStream> pStream;
	HRESULT hr = S_OK;

	if (pDevice == NULL) { std::cout << "pDevice is NULL\n"; std::cout.flush(); return false; }
	if (!openDevice(deviceID)) {
		std::cout << "could not open the device\n"; std::cout.flush();
		return false;
	}
	hr = pContent->Transfer(&pResources);
	if (FAILED(hr))	{
		printf("pContent->Transfer(&pResources) failed, hr = 0x%lx\n", hr); std::cout.flush();
		return false;
	}
	hr = pResources->GetStream(objectID,             // Identifier of the object we want to transfer
		WPD_RESOURCE_DEFAULT,    // We are transferring the default resource (which is the entire object's data)
		STGM_READ,               // Opening a stream in READ mode, because we are reading data from the device.
		optimalTransferSize,  // Driver supplied optimal transfer size
		&pStream);
	if (hr == E_WPD_DEVICE_NOT_OPEN) {
		/* device was probably unplugged since last call */
		openDevice(deviceID, true);
		hr = pResources->GetStream(objectID,             // Identifier of the object we want to transfer
			WPD_RESOURCE_DEFAULT,    // We are transferring the default resource (which is the entire object's data)
			STGM_READ,               // Opening a stream in READ mode, because we are reading data from the device.
			optimalTransferSize,  // Driver supplied optimal transfer size
			&pStream);
	}
	if (FAILED(hr))	{
		printf("pResources->GetStream failed, hr = 0x%lx\n", hr); std::cout.flush();
		return false;
	}
	hr = pStream->QueryInterface(IID_IPortableDeviceDataStream, (void **)&pDataStream);
	return true;
}


__declspec(dllexport) bool WPDI_readNextData(BYTE * data, DWORD size, DWORD *read) {
	HRESULT hr = pDataStream->Read(data, size, read);
	if (*read == 0) return false;
	return SUCCEEDED(hr);
}

__declspec(dllexport) void WPDI_CloseTransfer(void) {
	HRESULT hr;
	/*
	if (SUCCEEDED(hr) && pDataStream != NULL) {
		printf("pStream->QueryInterface succeeded, hr = 0x%lx\n", hr); std::cout.flush();
		//hr = pDataStream->Cancel();
		if (!SUCCEEDED(hr)) { printf("pDataStream->Cancel failed, hr = 0x%lx\n", hr); std::cout.flush(); }
		//hr = pDataStream->Release();
		if (!SUCCEEDED(hr)) { printf("pDataStream->Release failed, hr = 0x%lx\n", hr); std::cout.flush(); }
	}
	else {
		printf("pStream->QueryInterface failed, hr = 0x%lx\n", hr); std::cout.flush();
		hr = pStream->Release();
		if (!SUCCEEDED(hr)) { printf("pStream->Release failed, hr = 0x%lx\n", hr); std::cout.flush(); }
	}*/
	
	
	hr = pDataStream->Cancel();
	if (!SUCCEEDED(hr)) { printf("pStream->Cancel failed, hr = 0x%lx\n", hr); std::cout.flush(); }
	
	hr = pDataStream->Release();
	if (!SUCCEEDED(hr)) { printf("pStream->Release failed, hr = 0x%lx\n", hr); std::cout.flush(); }

	hr = pResources->Release();
	if (!SUCCEEDED(hr)) { printf("pResources->Release failed, hr = 0x%lx\n", hr); std::cout.flush(); }

	//delete pStream;
	//pStream = NULL;
}