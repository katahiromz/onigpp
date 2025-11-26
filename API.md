# Oniguruma API — Reference

Version 6.9.10 — 2024/06/26

```c
#include <oniguruma.h>
```

This document summarizes the primary Oniguruma C API (function signatures, arguments, return values and notable behaviors). It is derived from API.txt (English) and API_ja.txt (Japanese) in this repository.

## Initialization

```c
int onig_initialize(OnigEncoding use_encodings[], int num_encodings);
```
- Initialize library. Must be called explicitly.
- onig_init() is deprecated.
- Arguments:
  1. use_encodings: array of encodings used by the application.
  2. num_encodings: number of encodings.
- Return: ONIG_NORMAL (0) on success; error code (< 0) on failure.

## Error handling

```c
int onig_error_code_to_str(UChar* err_buf, int err_code, ...);
```
- Get error message into err_buf. If used for onig_new(), call before freeing the pattern string.
- err_buf must be at least ONIG_MAX_ERROR_MESSAGE_LEN.
- Returns the error message length (bytes).

## Warnings

```c
void onig_set_warn_func(OnigWarnFunc func);
void onig_set_verb_warn_func(OnigWarnFunc func);
```
- Set warning and verbose warning handlers respectively.
- Function signature: void (*func)(char* warning_message).

## Regex object creation / compilation

```c
int onig_new(regex_t** reg, const UChar* pattern, const UChar* pattern_end,
             OnigOptionType option, OnigEncoding enc, OnigSyntaxType* syntax,
             OnigErrorInfo* err_info);
```
- Create/compile a regex object.
- Arguments:
  1. reg: out pointer to regex_t*.
  2. pattern: pattern string.
  3. pattern_end: end address (pattern + length).
  4. option: compile-time options (see list below).
  5. enc: encoding (see list below).
  6. syntax: pointer to syntax definition (ONIG_SYNTAX_*).
  7. err_info: optional address for compile error info (used by onig_error_code_to_str()).
- Return: ONIG_NORMAL (0) on success; error code (< 0) on failure.

Important compile-time options (OnigOptionType):
- ONIG_OPTION_NONE
- ONIG_OPTION_SINGLELINE
- ONIG_OPTION_MULTILINE
- ONIG_OPTION_IGNORECASE
- ONIG_OPTION_EXTEND
- ONIG_OPTION_FIND_LONGEST
- ONIG_OPTION_FIND_NOT_EMPTY
- ONIG_OPTION_NEGATE_SINGLELINE
- ONIG_OPTION_DONT_CAPTURE_GROUP
- ONIG_OPTION_CAPTURE_GROUP
- ONIG_OPTION_IGNORECASE_IS_ASCII
- ONIG_OPTION_WORD_IS_ASCII
- ONIG_OPTION_DIGIT_IS_ASCII
- ONIG_OPTION_SPACE_IS_ASCII
- ONIG_OPTION_POSIX_IS_ASCII
- ONIG_OPTION_TEXT_SEGMENT_EXTENDED_GRAPHEME_CLUSTER
- ONIG_OPTION_TEXT_SEGMENT_WORD

(Note: ONIG_OPTION_FIND_LONGEST has limited behavior during backward search in onig_search())

Supported encodings (OnigEncoding), examples:
- ONIG_ENCODING_ASCII, ONIG_ENCODING_UTF8, ONIG_ENCODING_UTF16_BE/LE, ONIG_ENCODING_UTF32_BE/LE, ONIG_ENCODING_EUC_JP, ONIG_ENCODING_SJIS, ONIG_ENCODING_BIG5, ONIG_ENCODING_GB18030, etc.
- Or a user-defined OnigEncodingType address.

Supported syntaxes (OnigSyntaxType*): ONIG_SYNTAX_ASIS, ONIG_SYNTAX_POSIX_BASIC, ONIG_SYNTAX_POSIX_EXTENDED, ONIG_SYNTAX_EMACS, ONIG_SYNTAX_GREP, ONIG_SYNTAX_GNU_REGEX, ONIG_SYNTAX_JAVA, ONIG_SYNTAX_PERL, ONIG_SYNTAX_PERL_NG, ONIG_SYNTAX_PYTHON, ONIG_SYNTAX_ONIGURUMA, etc. ONIG_SYNTAX_DEFAULT == ONIG_SYNTAX_ONIGURUMA.

Alternative constructors:

```c
int onig_new_without_alloc(regex_t* reg, ...);
int onig_new_deluxe(regex_t** reg, const UChar* pattern, const UChar* pattern_end,
                    OnigCompileInfo* ci, OnigErrorInfo* einfo);
```
- onig_new_without_alloc: uses caller-allocated regex_t area.
- onig_new_deluxe: deprecated; does not allow differing pattern/target encodings in most cases. `ci` supplies compile-time info (num_of_elements, pattern_enc, target_enc, syntax, option, case_fold_flag).

## Freeing regex objects

```c
void onig_free(regex_t* reg);
void onig_free_body(regex_t* reg);
```
- onig_free(): free memory used by regex object.
- onig_free_body(): free internal memory used by regex object, but not the `reg` struct itself.

## Match parameters and limits

```c
OnigMatchParam* onig_new_match_param(void);
void onig_free_match_param(OnigMatchParam* mp);
void onig_initialize_match_param(OnigMatchParam* mp);
int onig_set_match_stack_limit_size_of_match_param(OnigMatchParam* mp, unsigned int limit);
int onig_set_retry_limit_in_match_of_match_param(OnigMatchParam* mp, unsigned long limit);
int onig_set_retry_limit_in_search_of_match_param(OnigMatchParam* mp, unsigned long limit);
int onig_set_progress_callout_of_match_param(OnigMatchParam* mp, OnigCalloutFunc f);
int onig_set_retraction_callout_of_match_param(OnigMatchParam* mp, OnigCalloutFunc f);
```
- Provide match-parameter management, including stack limits, retry limits and callouts. `0` generally means unlimited.
- Callout functions may be set to NULL to disable.

## Search and match

```c
int onig_search(regex_t* reg, const UChar* str, const UChar* end,
                const UChar* start, const UChar* range, OnigRegion* region,
                OnigOptionType option);

int onig_search_with_param(..., OnigMatchParam* mp);

int onig_match(regex_t* reg, const UChar* str, const UChar* end,
               const UChar* at, OnigRegion* region, OnigOptionType option);

int onig_match_with_param(..., OnigMatchParam* mp);
```
- onig_search: search for pattern in `str` within [start, range). Return match position offset (>=0) on success, ONIG_MISMATCH (<0) if not found, or error code (<0).
  - If option ONIG_OPTION_CALLBACK_EACH_MATCH is used, search returns ONIG_MISMATCH even if matches occur; use callbacks to enumerate matches.
  - Options (search-time): ONIG_OPTION_NOTBOL, ONIG_OPTION_NOTEOL, ONIG_OPTION_NOT_BEGIN_STRING, ONIG_OPTION_NOT_END_STRING, ONIG_OPTION_NOT_BEGIN_POSITION, ONIG_OPTION_CALLBACK_EACH_MATCH, ONIG_OPTION_MATCH_WHOLE_STRING.
- onig_match: attempt match at exact position `at`. Returns matched byte length (>=0), ONIG_MISMATCH if not matched, or error (<0).

## Scan

```c
int onig_scan(regex_t* reg, const UChar* str, const UChar* end,
              OnigRegion* region, OnigOptionType option,
              int (*scan_callback)(int, int, OnigRegion*, void*),
              void* callback_arg);
```
- Scan the string and invoke `scan_callback` for each match. Returns number of matches, error, or interruption (non-zero value returned by callback).

## Regset (multiple regex set) APIs

```c
int onig_regset_new(OnigRegSet** rset, int n, regex_t* regs[]);
int onig_regset_add(OnigRegSet* set, regex_t* reg);
int onig_regset_replace(OnigRegSet* set, int at, regex_t* reg);
void onig_regset_free(OnigRegSet* set);
int onig_regset_number_of_regex(OnigRegSet* set);
regex_t* onig_regset_get_regex(OnigRegSet* set, int at);
OnigRegion* onig_regset_get_region(OnigRegSet* set, int at);
int onig_regset_search(OnigRegSet* set, const OnigUChar* str, const OnigUChar* end,
                       const OnigUChar* start, const OnigUChar* range,
                       OnigRegSetLead lead, OnigOptionType option, int* rmatch_pos);
int onig_regset_search_with_param(...);
```
- Regset enforces that all regex objects have the same encoding and none use ONIG_OPTION_FIND_LONGEST.
- onig_regset_search returns index of matching regex on success; ONIG_MISMATCH if none; or an error code.

## Regions and captures

```c
OnigRegion* onig_region_new(void);
void onig_region_free(OnigRegion* region, int free_self);
void onig_region_copy(OnigRegion* to, OnigRegion* from);
void onig_region_clear(OnigRegion* region);
int onig_region_resize(OnigRegion* region, int n);
```
- Manage match region structures and captured group ranges.

## Name/group utilities

```c
int onig_name_to_group_numbers(regex_t* reg, const UChar* name, const UChar* name_end, int** num_list);
int onig_name_to_backref_number(regex_t* reg, const UChar* name, const UChar* name_end, OnigRegion* region);
int onig_foreach_name(regex_t* reg,
      int (*func)(const UChar*, const UChar*, int, int*, regex_t*, void*),
      void* arg);
int onig_number_of_names(regex_t* reg);
```
- Work with named groups and name-to-number mappings. `onig_name_to_group_numbers` returns number of groups for a given name or ONIGERR_UNDEFINED_NAME_REFERENCE if not found.
- `onig_name_to_backref_number` returns group number for named backref; uses `region` to determine which groups are active when multiple groups share the name.

## Getters / metadata

```c
OnigEncoding     onig_get_encoding(regex_t* reg);
OnigOptionType   onig_get_options(regex_t* reg);
OnigSyntaxType*  onig_get_syntax(regex_t* reg);
OnigCaseFoldType onig_get_case_fold_flag(regex_t* reg); /* deprecated */
int onig_number_of_captures(regex_t* reg);
OnigCallbackEachMatchFunc onig_get_callback_each_match(void);
int onig_set_callback_each_match(OnigCallbackEachMatchFunc func);
int onig_number_of_capture_histories(regex_t* reg);
OnigCaptureTreeNode* onig_get_capture_tree(OnigRegion* region);
int onig_capture_tree_traverse(OnigRegion* region, int at,
                               int(*func)(int,int,int,int,int,void*), void* arg);
int onig_noname_group_capture_is_active(regex_t* reg);
```
- Various introspection and callback setters. See API.txt for expanded semantics of capture histories and traversal callbacks.

## Encoding helpers

```c
UChar* onigenc_get_prev_char_head(OnigEncoding enc, const UChar* start, const UChar* s);
UChar* onigenc_get_left_adjust_char_head(OnigEncoding enc, const UChar* start, const UChar* s);
UChar* onigenc_get_right_adjust_char_head(OnigEncoding enc, const UChar* start, const UChar* s);
int onigenc_strlen(OnigEncoding enc, const UChar* s, const UChar* end);
int onigenc_strlen_null(OnigEncoding enc, const UChar* s);
int onigenc_str_bytelen_null(OnigEncoding enc, const UChar* s);
```
- Provide encoding-aware character boundary and length utilities. Do not pass invalid byte strings for the encoding.

## Syntax/encoding utilities

```c
int onig_set_default_syntax(OnigSyntaxType* syntax);
void onig_copy_syntax(OnigSyntaxType* to, OnigSyntaxType* from);
unsigned int onig_get_syntax_op(OnigSyntaxType* syntax);
unsigned int onig_get_syntax_op2(OnigSyntaxType* syntax);
unsigned int onig_get_syntax_behavior(OnigSyntaxType* syntax);
OnigOptionType onig_get_syntax_options(OnigSyntaxType* syntax);
void onig_set_syntax_op(OnigSyntaxType* syntax, unsigned int op);
void onig_set_syntax_op2(OnigSyntaxType* syntax, unsigned int op2);
void onig_set_syntax_behavior(OnigSyntaxType* syntax, unsigned int behavior);
void onig_set_syntax_options(OnigSyntaxType* syntax, OnigOptionType options);
void onig_copy_encoding(OnigEncoding to, OnigEncoding from);
```

```c
int onig_set_meta_char(OnigSyntaxType* syntax, unsigned int what, OnigCodePoint code);
```
- Set variable meta-character values (if the syntax enables variable meta characters). Returns ONIG_NORMAL on success. `what` is one of ONIG_META_CHAR_ESCAPE, ONIG_META_CHAR_ANYCHAR, etc. Set to ONIG_INEFFECTIVE_META_CHAR to disable.

## Case-fold flag (deprecated)

```c
OnigCaseFoldType onig_get_default_case_fold_flag(void); /* deprecated */
int onig_set_default_case_fold_flag(OnigCaseFoldType case_fold_flag); /* deprecated */
```

## Global limits and retry configuration

```c
unsigned int onig_get_match_stack_limit_size(void);
int onig_set_match_stack_limit_size(unsigned int size); /* size = 0 -> unlimited */

unsigned long onig_get_retry_limit_in_match(void);
int onig_set_retry_limit_in_match(unsigned long limit); /* 0 -> unlimited */

unsigned long onig_get_retry_limit_in_search(void);
int onig_set_retry_limit_in_search(unsigned long limit); /* 0 -> unlimited */
```
- Defaults and setters for match stack limits and retry limits. onig_get_retry_limit_in_match default: 10000000; onig_get_retry_limit_in_search default: 0.

## Notes and behavioral details

- Many functions return ONIG_NORMAL (0) on success and negative error codes on failure. Some APIs return ONIG_MISMATCH (< 0) to indicate no match.
- Do not pass invalid byte sequences for the specified encoding to Oniguruma functions.
- Consult the corresponding API.txt (English) and API_ja.txt (Japanese) files for the complete, line-by-line reference and subtle behaviors not fully reproduced here (capture-history details, exact error codes, and all enums).
- Capture history / capture-tree APIs are available when the syntax enables capture-history (ONIG_SYN_OP2_ATMARK_CAPTURE_HISTORY). Refer to API.txt for traversal callback prototypes and semantics.

## onigpp C++ Wrapper Flags

The onigpp C++ wrapper provides the following `syntax_option_type` flags:

- `ECMAScript` (default): Use ECMAScript grammar.
- `basic`, `extended`, `awk`, `grep`, `egrep`: POSIX and other standard grammars.
- `icase`: Case-insensitive matching.
- `multiline`: ECMAScript multiline behavior emulation.
- `nosubs`: Do not store submatch results.
- `optimize`: Compilation optimization hints (implementation-specific).
- `collate`: Locale-dependent collation.
- `oniguruma`: Enable Oniguruma's native syntax and behavior.

## See also

- API.txt (this repository)
- API_ja.txt (this repository)
- RE.md / RE.txt for regular-expression syntax details
- Unicode UAX #29 (grapheme cluster and word boundary rules): http://unicode.org/reports/tr29/