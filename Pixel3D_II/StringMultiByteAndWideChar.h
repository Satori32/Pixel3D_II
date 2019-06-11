#pragma once
#include <cstdlib>
#include <string>
#include <string_view>
#include <clocale>

bool SetLocaleJapanese() {
	//std::setlocale(LC_CTYPE,"Japanese");
	std::setlocale(LC_CTYPE,"");
	return true;
}

std::string WideCharToMultiByte(const std::wstring& W) {
	std::size_t L = W.length() * MB_CUR_MAX + 1;
	char* MB = new char[L];
	wcstombs_s(nullptr, MB, L,W.data(), L);
	std::string S = MB;
	delete[] MB;
	return S;
}

std::wstring MultiByteToWideChar(const std::string& S) {
	std::size_t L = S.length() + 1;
	wchar_t* WC = new wchar_t[L];
	mbstowcs_s(nullptr,WC, MB_CUR_MAX,S.data(), L);
	std::wstring W = WC;
	delete[] WC;

	return W;
	
}
