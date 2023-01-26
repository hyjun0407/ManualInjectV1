#include "ManualMap.h"
#include <random>
#pragma warning(disable : 4996)
#include <winhttp.h>
#include <direct.h>
#include "VMProtectSDK.h"
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "winhttp.lib")

bool DisableProxies(void)
{
	HKEY hKey;
	DWORD data = 0;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"), 0, KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS)
		return false; // Key could not be opened

	if (RegSetValueExW(hKey, L"ProxyEnable", 0, REG_DWORD, (LPBYTE)&data, sizeof(data)) != ERROR_SUCCESS)
		return false; // Failed to set the value

	if (RegCloseKey(hKey) != ERROR_SUCCESS)
		return false; // Could not close key.

	return true;
}

wstring get_utf16(const string& str, int codepage)
{
	if (str.empty()) return wstring();
	int sz = MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), 0, 0);
	wstring res(sz, 0);
	MultiByteToWideChar(codepage, 0, &str[0], (int)str.size(), &res[0], sz);
	return res;
}

string get_system_uuid()
{
	if (system("wmic csproduct get uuid > HWID.txt") == 0)
	{
		auto file = ::fopen("HWID.txt", "rt, ccs=UNICODE"); // open the file for unicode input

		enum { BUFFSZ = 1000, UUID_SZ = 36 };
		wchar_t wbuffer[BUFFSZ]; // buffer to hold unicode characters

		if (file && // file was succesffully opened
			::fgetws(wbuffer, BUFFSZ, file) && // successfully read (and discarded) the first line
			::fgetws(wbuffer, BUFFSZ, file)) // yfully read the second line
		{
			char cstr[BUFFSZ]; // buffer to hold the converted c-style string
			if (::wcstombs(cstr, wbuffer, BUFFSZ) > UUID_SZ) // convert unicode to utf-8
			{
				string uuid = cstr;
				while (!uuid.empty() && isspace(uuid.back())) uuid.pop_back(); // discard trailing white space
				return uuid;
			}
		}
	}
	return {}; // failed, return empty string
}

string WebWinhttp(string details)
{
	VMProtectBeginUltra("WEBFUNC_");
	DWORD dwSize = 0, dwDownloaded;
	LPSTR source;
	source = (char*)"";
	string responsed = "";

	HINTERNET hSession = NULL, hConnect = NULL, hRequest = NULL;
	BOOL bResults = FALSE;

	hSession = WinHttpOpen(L"Winhttp API", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

	if (hSession)
		hConnect = WinHttpConnect(hSession, get_utf16("core24.dothome.co.kr", CP_UTF8).c_str(), INTERNET_DEFAULT_HTTP_PORT, 0);

	if (hConnect)
		hRequest = WinHttpOpenRequest(hConnect, L"GET", get_utf16(details, CP_UTF8).c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);

	if (hRequest)
		bResults = WinHttpSendRequest(hRequest, L"content-type:application/x-www-form-urlencoded", -1, WINHTTP_NO_REQUEST_DATA, 0, 0, 0);

	if (bResults)
		bResults = WinHttpReceiveResponse(hRequest, NULL);

	if (bResults) {
		do {
			dwSize = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
				printf("Error %u", GetLastError());

			source = (char*)malloc(dwSize + 1);
			if (!source) {
				printf("Out of memory\n");
				dwSize = 0;
			}
			else {
				std::memset(source, 0, dwSize + 1);

				if (!WinHttpReadData(hRequest, (LPVOID)source, dwSize, &dwDownloaded))
					printf("Error %u", GetLastError());
				else
					responsed = responsed + source;
				free(source);
			}
		} while (dwSize > 0);
	}

	if (!bResults) {
		exit(0);
	}

	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);
	return responsed;
	VMProtectEnd();
}

BOOL load_driver(string TargetDriver, string TargetServiceName, string TargetServiceDesc)
{
	SC_HANDLE ServiceManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (!ServiceManager) return FALSE;
	SC_HANDLE ServiceHandle = CreateService(ServiceManager, TargetServiceName.c_str(), TargetServiceDesc.c_str(), SERVICE_START | DELETE | SERVICE_STOP, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE, TargetDriver.c_str(), NULL, NULL, NULL, NULL, NULL);
	if (!ServiceHandle)
	{
		ServiceHandle = OpenService(ServiceManager, TargetServiceName.c_str(), SERVICE_START | DELETE | SERVICE_STOP);
		if (!ServiceHandle) return FALSE;
	}
	if (!StartServiceA(ServiceHandle, NULL, NULL)) return FALSE;
	CloseServiceHandle(ServiceHandle);
	CloseServiceHandle(ServiceManager);
	return TRUE;
}

BOOL delete_service(string TargetServiceName)
{
	SERVICE_STATUS ServiceStatus;
	SC_HANDLE ServiceManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
	if (!ServiceManager) return FALSE;
	SC_HANDLE ServiceHandle = OpenService(ServiceManager, TargetServiceName.c_str(), SERVICE_STOP | DELETE);
	if (!ServiceHandle) return FALSE;
	if (!ControlService(ServiceHandle, SERVICE_CONTROL_STOP, &ServiceStatus)) return FALSE;
	if (!DeleteService(ServiceHandle)) return FALSE;
	CloseServiceHandle(ServiceHandle);
	CloseServiceHandle(ServiceManager);
	return TRUE;
}

std::string random_string(std::string::size_type length)
{
	const char *chrs = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	thread_local static std::mt19937 rg{ std::random_device{}() };
	thread_local static std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);
	std::string s;
	s.reserve(length);
	while (length--)
		s += chrs[pick(rg)];
	return s;
}

int main()
{

	cout << "Login Success" << endl;
	Sleep(2500);
	system("taskkill /im Loader.exe");
	remove("C:\\Windows\\System32\\drivers\\Manager");
	Sleep(500);
	_mkdir("C:\\Windows\\System32\\drivers\\Manager");
	URLDownloadToFile(NULL, "https://github.com/katlogic/WindowsD/releases/download/v2.2/wind64.exe", "C:\\Windows\\System32\\drivers\\Manager\\Loader.exe", 0, NULL);
	URLDownloadToFile(NULL, "http://core24.dothome.co.kr/LAST/faiophfasfh13/LAST.sys", "C:\\Windows\\System32\\drivers\\Manager\\flkahsfugawbn13.sys", 0, NULL);
	URLDownloadToFile(NULL, "http://core24.dothome.co.kr/LAST/faiophfasfh13/LAST.dll", "C:\\Windows\\System32\\drivers\\Manager\\fjopasjfshn13.dll", 0, NULL);

	ShellExecute(0, "open", "C:\\Windows\\System32\\drivers\\Manager\\Loader.exe", " -i", 0, SW_HIDE);
	string service_name = random_string(15);
	load_driver("C:\\Windows\\System32\\drivers\\Manager\\flkahsfugawbn13.sys", service_name, service_name);
	while (!mem->Init("TslGame.exe"))
		Sleep(1);
	while (mem->RPM<int>(mem->ModuleBase) != 9460301)
		Sleep(1);
	ManualMap("C:\\Windows\\System32\\drivers\\Manager\\fjopasjfshn13.dll");
	CloseHandle(mem->hDevice);
	delete_service(service_name);
	ShellExecute(0, "open", "C:\\Windows\\System32\\drivers\\Manager\\Loader.exe", " -u", 0, SW_HIDE);
	system("taskkill /im Loader.exe");
	printf("Inject Success!");
	Sleep(100);
	exit(0);
}