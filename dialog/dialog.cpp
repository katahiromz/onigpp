// dialog.cpp
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <string>
#include "onigpp.h"

namespace rex = onigpp;

#ifdef UNICODE
	using string_type = std::wstring;
	using regex_type = rex::wregex;
#else
	using string_type = std::string;
	using regex_type = rex::regex;
#endif

bool process_one(const string_type& input, const string_type& replacement, string_type& out_str, regex_type& re) {
	try {
		out_str = rex::regex_replace(input, re, replacement);
		return true;
	} catch (const rex::regex_error& e) {
		return false;
	}
};

void OnReplace(HWND hwnd) {
	BOOL oniguruma = IsDlgButtonChecked(hwnd, chx1) == BST_CHECKED;
	BOOL ecma = IsDlgButtonChecked(hwnd, chx2) == BST_CHECKED;
	BOOL icase = IsDlgButtonChecked(hwnd, chx3) == BST_CHECKED;

	TCHAR text[4][256];
	GetDlgItemText(hwnd, edt1, text[0], _countof(text[0]));
	GetDlgItemText(hwnd, edt2, text[1], _countof(text[1]));
	GetDlgItemText(hwnd, edt3, text[2], _countof(text[2]));
	GetDlgItemText(hwnd, edt4, text[3], _countof(text[3]));
	string_type input = text[0];
	string_type pattern = text[2];
	string_type replacement = text[3];

	int flags = 0;
	if (oniguruma) flags |= rex::regex::oniguruma;
	if (ecma) flags |= rex::regex::ECMAScript;
	if (icase) flags |= rex::regex::icase;

	regex_type re;
	try {
		re = regex_type(pattern, flags);
	} catch (const rex::regex_error& e) {
		MessageBox(hwnd, TEXT("Failure!"), NULL, MB_ICONERROR);
		return;
	}

	string_type out_str;
	if (!process_one(input, replacement, out_str, re)) {
		MessageBox(hwnd, TEXT("Failure! #2"), NULL, MB_ICONERROR);
		return;
	}

	SetDlgItemText(hwnd, edt2, out_str.c_str());
}

// WM_INITDIALOG
BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {
	CheckDlgButton(hwnd, chx1, BST_CHECKED); // oniguruma
	return TRUE;
}

// WM_COMMAND
void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
	switch (id) {
	case IDOK:
	case IDCANCEL:
		EndDialog(hwnd, id);
		break;
	case psh1: // Replace
		OnReplace(hwnd);
		break;
	}
}

INT_PTR CALLBACK
DialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg) {
		HANDLE_MSG(hwnd, WM_INITDIALOG, OnInitDialog);
		HANDLE_MSG(hwnd, WM_COMMAND, OnCommand);
	}
	return 0;
}

INT WINAPI
WinMain(HINSTANCE   hInstance,
        HINSTANCE   hPrevInstance,
        LPSTR       lpCmdLine,
        INT         nCmdShow)
{
	onigpp::auto_init init;

	InitCommonControls();

	DialogBox(hInstance, MAKEINTRESOURCE(1), NULL, DialogProc);
	return 0;
}
