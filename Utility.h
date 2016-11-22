#pragma once

using namespace std;

class Utility
{
public:
	static BOOL IsWinXP()
	{
		OSVERSIONINFO version;
		version.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		::GetVersionEx(&version);
		return version.dwMajorVersion <= 5;
	}
	static std::wstring UTF8ToWString(const char* lpcszString)
	{
		int len = strlen(lpcszString);
		int unicodeLen = ::MultiByteToWideChar(CP_UTF8, 0, lpcszString, -1, NULL, 0);
		wchar_t* pUnicode;
		pUnicode = new wchar_t[unicodeLen + 1];
		memset((void*)pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
		::MultiByteToWideChar(CP_UTF8, 0, lpcszString, -1, (LPWSTR)pUnicode, unicodeLen);
		wstring wstrReturn(pUnicode);
		delete[] pUnicode;
		return wstrReturn;
	}

	static std::string WStringToUTF8(const wchar_t* lpwcszWString)
	{
		char* pElementText;
		int iTextLen = ::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)lpwcszWString, -1, NULL, 0, NULL, NULL);
		pElementText = new char[iTextLen + 1];
		memset((void*)pElementText, 0, (iTextLen + 1) * sizeof(char));
		::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)lpwcszWString, -1, pElementText, iTextLen, NULL, NULL);
		std::string strReturn(pElementText);
		delete[] pElementText;
		return strReturn;
	}

	static std::wstring MBytesToWString(const char* lpcszString)
	{
		int len = strlen(lpcszString);
		int unicodeLen = ::MultiByteToWideChar(CP_ACP, 0, lpcszString, -1, NULL, 0);
		wchar_t* pUnicode = new wchar_t[unicodeLen + 1];
		memset(pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
		::MultiByteToWideChar(CP_ACP, 0, lpcszString, -1, (LPWSTR)pUnicode, unicodeLen);
		wstring wString = (wchar_t*)pUnicode;
		delete[] pUnicode;
		return wString;
	}

	static std::string WStringToMBytes(const wchar_t* lpwcszWString)
	{
		char* pElementText;
		int iTextLen;
		// wide char to multi char
		iTextLen = ::WideCharToMultiByte(CP_ACP, 0, lpwcszWString, -1, NULL, 0, NULL, NULL);
		pElementText = new char[iTextLen + 1];
		memset((void*)pElementText, 0, (iTextLen + 1) * sizeof(char));
		::WideCharToMultiByte(CP_ACP, 0, lpwcszWString, 0, pElementText, iTextLen, NULL, NULL);
		std::string strReturn(pElementText);
		delete[] pElementText;
		return strReturn;
	}
	template<typename dst_type, typename src_type>
	static dst_type Pointer_Cast(src_type src)
	{
		return *static_cast<dst_type*>(static_cast<void*>(&src));
	}
};