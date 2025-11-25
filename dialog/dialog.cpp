// dialog.cpp --- Oniguruma++ test dialog
// Author: katahiromz
// License: BSD-2-Clause
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <string>
#include <tchar.h>
#include "onigpp.h"

namespace rex = onigpp;

#ifdef UNICODE
	using string_type = std::wstring;
	using char_type = wchar_t;
	using regex_type = rex::wregex;
#else
	using string_type = std::string;
	using char_type = char;
	using regex_type = rex::regex;
#endif

// 共通ヘルパー関数群を匿名名前空間にまとめる
namespace {

using size_type = typename string_type::size_type;
using const_iter = typename string_type::const_iterator;

string_type mstr_unescape(const string_type& input) {
	string_type output;
	size_t i = 0;
	while (i < input.size()) {
		if (input[i] == char_type('\\') && i + 1 < input.size()) {
			char_type next = input[i + 1];
			switch (next) {
			case char_type('n'):  output += char_type('\n'); break;
			case char_type('t'):  output += char_type('\t'); break;
			case char_type('r'):  output += char_type('\r'); break;
			case char_type('b'):  output += char_type('\b'); break;
			case char_type('f'):  output += char_type('\f'); break;
			case char_type('a'):  output += char_type('\a'); break;
			case char_type('v'):  output += char_type('\v'); break;
			case char_type('\\'): output += char_type('\\'); break;
			case char_type('\''): output += char_type('\''); break;
			case char_type('\"'): output += char_type('\"'); break;
			case char_type('?'):  output += char_type('\?'); break;
			case char_type('x'): { // \xXX 形式
				i += 2;
				string_type hex;
				while (i < input.size() && _istxdigit(input[i])) {
					hex += input[i++];
				}
				output += hex.empty() ? char_type('x') : char_type(_tcstoul(hex.c_str(), nullptr, 16));
				i--;
				break;
			}
			case char_type('u'): { // \uXXXX 形式
				i += 2;
				string_type hex;
				size_t index = 0;
				while (index < 4 && i < input.size() && _istxdigit(input[i])) {
					hex += input[i++];
					++index;
				}
				if (index == 4) {
					output += hex.empty() ? char_type('x') : char_type(_tcstoul(hex.c_str(), nullptr, 16));
					i--;
				} else {
					throw std::runtime_error("Invalid escape sequence");
				}
				break;
			}
			default:
				// 8進数（\123）対応
				if (next >= char_type('0') && next <= char_type('7')) {
					size_t j = i + 1;
					string_type oct;
					for (int k = 0; k < 3 && j < input.size() && input[j] >= char_type('0') && input[j] <= char_type('7'); ++k, ++j) {
						oct += input[j];
					}
					output += oct.empty() ? next : char_type(_tcstoul(oct.c_str(), nullptr, 8));
					i += oct.size();
				} else {
					// その他はそのまま
					output += next;
				}
				break;
			}
			i += 2;
		} else {
			output += input[i++];
		}
	}
	return output;
}

// start_from (文字位置) から検索して、見つかれば match_pos/match_len を返す。
// start_from > input.size() は末尾とみなす。見つからなければ先頭からラップして検索する。
// return: true = 見つかった, false = 見つからなかった
bool find_next_match(const string_type& input, size_type start_from, size_type& match_pos, size_type& match_len, regex_type& re) {
	try {
		const size_type input_len = input.size();
		if (start_from > input_len) start_from = input_len;

		rex::match_results<const_iter> m;
		const_iter begin = input.begin();
		const_iter end = input.end();

		// search from start_from .. end
		const_iter start_it = begin + static_cast<ptrdiff_t>(start_from);
		if (rex::regex_search(start_it, end, m, re)) {
			match_pos = start_from + static_cast<size_type>(m.position(0));
			match_len = static_cast<size_type>(m.length(0));
			return true;
		}

		// wrap-around: search from beginning
		if (input_len > 0) {
			if (rex::regex_search(begin, end, m, re)) {
				match_pos = static_cast<size_type>(m.position(0));
				match_len = static_cast<size_type>(m.length(0));
				return true;
			}
		}
		return false;
	} catch (const rex::regex_error&) {
		return false;
	}
}

// 現在の選択範囲 [sel_start, sel_end) が正確に一つのマッチと等しいか判定する
bool selection_is_exact_match(const string_type& input, size_type sel_start, size_type sel_end, regex_type& re) {
	if (sel_start >= sel_end) return false;
	const_iter first = input.begin() + static_cast<ptrdiff_t>(sel_start);
	const_iter last = input.begin() + static_cast<ptrdiff_t>(sel_end);
	rex::match_results<const_iter> m;
	try {
		if (rex::regex_search(first, last, m, re)) {
			// マッチが選択範囲の先頭にあり、長さが選択範囲と同じなら一致とする
			return (m.position(0) == 0) && (static_cast<size_type>(m.length(0)) == (sel_end - sel_start));
		}
	} catch (const rex::regex_error&) {
		// regex エラーは false
	}
	return false;
}

// match_pos/match_len で示される単一のマッチを replacement で置換し、結果文字列を out に格納する。
// 置換された部分の長さ（バイト/文字長）を返す。
size_type perform_single_replacement(const string_type& input, size_type match_pos, size_type match_len, const string_type& replacement, regex_type& re, string_type& out) {
	const string_type prefix = input.substr(0, match_pos);
	const string_type matched = input.substr(match_pos, match_len);
	const string_type suffix = (match_pos + match_len <= input.size()) ? input.substr(match_pos + match_len) : string_type();

	// マッチ部分のみを置換（format_first_only を使い、キャプチャ参照などを正しく扱う）
	const string_type replaced_matched = rex::regex_replace(matched, re, replacement, rex::regex_constants::format_first_only);
	out = prefix + replaced_matched + suffix;
	return replaced_matched.size();
}

// 置換対象の件数を数える（ゼロ長マッチ対策：ゼロ長なら 1 文字進める）
size_type count_matches(const string_type& input, regex_type& re) {
	size_type count = 0;
	const_iter it = input.begin();
	const_iter end = input.end();
	rex::match_results<const_iter> m;

	while (it != end) {
		try {
			if (!rex::regex_search(it, end, m, re)) break;
		} catch (const rex::regex_error&) {
			return 0;
		}
		const size_type found_pos = static_cast<size_type>(m.position(0)) + static_cast<size_type>(it - input.begin());
		count++;
		const size_type advance = (m.length(0) > 0) ? static_cast<size_type>(m.length(0)) : 1;
		it = input.begin() + static_cast<ptrdiff_t>(found_pos + advance);
	}
	return count;
}

} // namespace

// do_find は find_next_match を利用して実装
bool do_find(const string_type& input, DWORD& iStart, DWORD& iEnd, regex_type& re) {
	try {
		size_type match_pos = 0, match_len = 0;
		const size_type search_from = static_cast<size_type>(iEnd);
		if (!find_next_match(input, search_from, match_pos, match_len, re)) return false;

		iStart = static_cast<DWORD>(match_pos);
		iEnd = static_cast<DWORD>(match_pos + match_len);
		return true;
	} catch (const rex::regex_error&) {
		return false;
	}
}

void OnFindReplace(HWND hwnd, int action) {
	BOOL unescape = IsDlgButtonChecked(hwnd, chx1) == BST_CHECKED;
	BOOL ecma = IsDlgButtonChecked(hwnd, chx2) == BST_CHECKED;
	BOOL icase = IsDlgButtonChecked(hwnd, chx3) == BST_CHECKED;
	BOOL multiline = IsDlgButtonChecked(hwnd, chx4) == BST_CHECKED;

	TCHAR input_text[256], pattern_text[256], replacement_text[256];
	GetDlgItemText(hwnd, edt1, input_text, _countof(input_text));
	GetDlgItemText(hwnd, edt3, pattern_text, _countof(pattern_text));
	GetDlgItemText(hwnd, edt4, replacement_text, _countof(replacement_text));
	string_type input = input_text;
	string_type pattern = pattern_text;
	string_type replacement = replacement_text;

	int flags = 0;
	if (unescape) replacement = mstr_unescape(replacement);
	if (ecma) flags |= rex::regex::ECMAScript;
	if (icase) flags |= rex::regex::icase;
	if (multiline) flags |= rex::regex::multiline;

	regex_type re;
	try {
		re = regex_type(pattern, flags);
	} catch (const rex::regex_error&) {
		MessageBox(hwnd, TEXT("Failure!"), NULL, MB_ICONERROR);
		return;
	}

	DWORD iStart, iEnd;
	SendDlgItemMessage(hwnd, edt1, EM_GETSEL, (WPARAM)&iStart, (LPARAM)&iEnd);

	const size_type input_len = input.size();
	if (iStart > input_len) iStart = static_cast<DWORD>(input_len);
	if (iEnd > input_len) iEnd = static_cast<DWORD>(input_len);

	switch (action) {
	case 0: // find
		{
			if (!do_find(input, iStart, iEnd, re)) {
				MessageBox(hwnd, TEXT("No more match"), TEXT("dialog"), MB_ICONINFORMATION);
				return;
			}
		}
		break;
	case 1: // replace (Windows 標準 UX に従う)
		{
			try {
				size_type match_pos = 0, match_len = 0;
				bool found = false;

				// 1) 現在の選択がちょうどマッチならそれを置換
				if (selection_is_exact_match(input, static_cast<size_type>(iStart), static_cast<size_type>(iEnd), re)) {
					found = true;
					match_pos = static_cast<size_type>(iStart);
					match_len = static_cast<size_type>(iEnd - iStart);
				}

				// 2) そうでなければ選択の終端から次を検索（ラップあり）
				if (!found) {
					const size_type start_from = static_cast<size_type>(iEnd);
					if (find_next_match(input, start_from, match_pos, match_len, re)) {
						found = true;
					}
				}

				if (!found) {
					MessageBox(hwnd, TEXT("No more match"), TEXT("dialog"), MB_ICONINFORMATION);
					return;
				}

				// 実際の置換（単一マッチ）
				string_type out;
				const size_type replaced_len = perform_single_replacement(input, match_pos, match_len, replacement, re, out);

				// 更新
				SetDlgItemText(hwnd, edt1, out.c_str());

				// 選択を置換後の領域にセット
				iStart = static_cast<DWORD>(match_pos);
				iEnd = static_cast<DWORD>(match_pos + replaced_len);

				// 編集コントロール内のテキスト変化を反映するため、input を更新しても良いが
				// 現在は SetDlgItemText で置換済み。EM_SETSEL は後で呼ばれる。
			} catch (const rex::regex_error&) {
				MessageBox(hwnd, TEXT("Failure!"), NULL, MB_ICONERROR);
				return;
			}
		}
		break;
	case 2: // replace all
		{
			try {
				// 置換対象件数を数える
				const size_type cnt = count_matches(input, re);
				if (cnt == 0) {
					MessageBox(hwnd, TEXT("No more match"), TEXT("dialog"), MB_ICONINFORMATION);
					return;
				}

				// 全置換
				const string_type out = rex::regex_replace(input, re, replacement);
				SetDlgItemText(hwnd, edt1, out.c_str());

				// 件数をユーザーに通知
#ifdef UNICODE
				std::wstring msg = std::to_wstring(cnt) + L" occurrences replaced.";
				MessageBox(hwnd, msg.c_str(), TEXT("dialog"), MB_OK | MB_ICONINFORMATION);
#else
				std::string msg = std::to_string(cnt) + " occurrences replaced.";
				MessageBox(hwnd, msg.c_str(), TEXT("dialog"), MB_OK | MB_ICONINFORMATION);
#endif
				// 選択解除
				iStart = iEnd = 0;
			} catch (const rex::regex_error&) {
				MessageBox(hwnd, TEXT("Failure!"), NULL, MB_ICONERROR);
				return;
			}
		}
		break;
	default:
		assert(0);
		return;
	}

	// 最後に選択をセット（Find/Replace/ReplaceAll 共通）
	SendDlgItemMessage(hwnd, edt1, EM_SETSEL, iStart, iEnd);
}

// WM_INITDIALOG
BOOL OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {
	SetDlgItemText(hwnd, edt1, TEXT("This is a test.\r\n\r\nThis is a test.\r\n"));
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
		OnFindReplace(hwnd, 1);
		break;
	case psh2: // Find
		OnFindReplace(hwnd, 0);
		break;
	case psh3: // Replace All
		OnFindReplace(hwnd, 2);
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
		LPSTR	   lpCmdLine,
		INT		 nCmdShow)
{
	onigpp::auto_init init;

	InitCommonControls();

	DialogBox(hInstance, MAKEINTRESOURCE(1), NULL, DialogProc);
	return 0;
}
