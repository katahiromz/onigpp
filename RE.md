# Oniguruma++ Regular Expressions

**Version 6.9.16** — 2025/11/27

---

## 1. Syntax elements

| Element | Description |
|---------|-------------|
| `\` | escape (enable or disable meta character) |
| `|` | alternation |
| `(...)` | group |
| `[...]` | character class |

---

## 2. Characters

### Character Escapes

| Escape | Description | Value |
|--------|-------------|-------|
| `\t` | horizontal tab | `0x09` |
| `\v` | vertical tab | `0x0B` |
| `\n` | newline (line feed) | `0x0A` |
| `\r` | carriage return | `0x0D` |
| `\b` | backspace | `0x08` |
| `\f` | form feed | `0x0C` |
| `\a` | bell | `0x07` |
| `\e` | escape | `0x1B` |
| `\nnn` | octal char | encoded byte value |
| `\xHH` | hexadecimal char | encoded byte value |
| `\x{7HHHHHHH}` | hexadecimal char (1-8 digits) | code point value |
| `\o{17777777777}` | octal char (1-11 digits) | code point value |
| `\uHHHH` | hexadecimal char | code point value |
| `\cx` | control char | code point value |
| `\C-x` | control char | code point value |
| `\M-x` | meta (x\|0x80) | code point value |
| `\M-\C-x` | meta control char | code point value |

> **Note:** `\b` as backspace is effective in character class only.

### 2.1 Code point sequences

**Hexadecimal code point** (1-8 digits):
```
\x{7HHHHHHH 7HHHHHHH ... 7HHHHHHH}
```

**Octal code point** (1-11 digits):
```
\o{17777777777 17777777777 ... 17777777777}
```

---

## 3. Character types

### Basic Character Types

| Pattern | Description |
|---------|-------------|
| `.` | any character (except newline) |
| `\w` | word character |
| `\W` | non-word char |
| `\s` | whitespace char |
| `\S` | non-whitespace char |
| `\d` | decimal digit char |
| `\D` | non-decimal-digit char |
| `\h` | hexadecimal digit char `[0-9a-fA-F]` |
| `\H` | non-hexdigit char |

### Word Character (`\w`)

**Not Unicode:**
- alphanumeric, "_" and multibyte char.

**Unicode:**
- General_Category — (Letter\|Mark\|Number\|Connector_Punctuation)

### Whitespace Character (`\s`)

**Not Unicode:**
- `\t`, `\n`, `\v`, `\f`, `\r`, `\x20`

**Unicode case:**
- U+0009, U+000A, U+000B, U+000C, U+000D, U+0085(NEL)
- General_Category — Line_Separator
- General_Category — Paragraph_Separator
- General_Category — Space_Separator

### Decimal Digit (`\d`)

**Unicode:** General_Category — Decimal_Number

### Special Newline and Text Operators

| Pattern | Description |
|---------|-------------|
| `\R` | general newline (can't be used in character-class) |
| `\N` | negative newline `(?-m:.)` |
| `\O` | true anychar `(?m:.)` (original function) |
| `\X` | Text Segment (see below) |

#### `\R` - General Newline

Matches `"\r\n"` or `\n`, `\v`, `\f`, `\r` (doesn't backtrack from `\r\n` to `\r`)

**Unicode case:** `"\r\n"` or `\n`, `\v`, `\f`, `\r` or U+0085, U+2028, U+2029

#### `\X` - Text Segment

Equivalent to: `\X === (?>\O(?:\Y\O)*)`

The meaning of this operator changes depending on the setting of the option `(?y{..})`.

`\X` doesn't check whether matching start position is boundary or not. Please write as `\y\X` if you want to ensure it.

**[Extended Grapheme Cluster mode]** (default)
- **Unicode case:** See [Unicode Standard Annex #29](http://unicode.org/reports/tr29/)
- **Not Unicode case:** `\X === (?>\r\n|\O)`

**[Word mode]**
- Currently, this mode is supported in Unicode only.
- See [Unicode Standard Annex #29](http://unicode.org/reports/tr29/)

### Character Property

Character properties can be specified using:

- `\p{property-name}`
- `\p{^property-name}` (negative)
- `\P{property-name}` (negative)
- `\pX` (X = C, L, M, N, P, S, Z)
- `\PX` (X = C, L, M, N, P, S, Z) (negative)

#### Property names

**Works on all encodings:**
- Alnum, Alpha, Blank, Cntrl, Digit, Graph, Lower, Print, Punct, Space, Upper, XDigit, Word, ASCII

**Works on EUC_JP, Shift_JIS:**
- Hiragana, Katakana

**Works on UTF8, UTF16, UTF32:**
- See doc/UNICODE_PROPERTIES.

---

## 4. Quantifier

### Greedy

| Quantifier | Description |
|------------|-------------|
| `?` | 1 or 0 times |
| `*` | 0 or more times |
| `+` | 1 or more times |
| `{n,m}` | at least n but no more than m times (n ≤ m) |
| `{n,}` | at least n times |
| `{,n}` | at least 0 but no more than n times (`{0,n}`) |
| `{n}` | n times |

### Reluctant

| Quantifier | Description |
|------------|-------------|
| `??` | 0 or 1 times |
| `*?` | 0 or more times |
| `+?` | 1 or more times |
| `{n,m}?` | at least n but not more than m times (n ≤ m) |
| `{n,}?` | at least n times |
| `{,n}?` | at least 0 but not more than n times (`{0,n}?`) |

> **Note:** `{n}?` is reluctant operator in `ONIG_SYNTAX_JAVA` and `ONIG_SYNTAX_PERL` only. (In that case, it doesn't make sense to write so.) In default syntax, `/a{n}?/` === `/(?:a{n})?/`

### Possessive (greedy and does not backtrack once match)

| Quantifier | Description |
|------------|-------------|
| `?+` | 1 or 0 times |
| `*+` | 0 or more times |
| `++` | 1 or more times |
| `{n,m}` | at least m but not more than n times (n > m) |

> **Note:** `{n,m}+`, `{n,}+`, `{n}+` are possessive operators in `ONIG_SYNTAX_JAVA` and `ONIG_SYNTAX_PERL` only.

---

## 5. Anchors

### Basic Anchors

| Anchor | Description |
|--------|-------------|
| `^` | beginning of the line |
| `$` | end of the line |
| `\b` | word boundary |
| `\B` | non-word boundary |
| `\A` | beginning of string |
| `\Z` | end of string, or before newline at the end |
| `\z` | end of string |
| `\G` | where the current search attempt begins |
| `\K` | keep (keep start position of the result string) |

### Text Segment Boundaries

| Anchor | Description |
|--------|-------------|
| `\y` | Text Segment boundary |
| `\Y` | Text Segment non-boundary |

The meaning of these operators (`\y`, `\Y`) changes depending on the setting of the option `(?y{..})`.

**[Extended Grapheme Cluster mode]** (default)
- **Unicode case:** See [Unicode Standard Annex #29](http://unicode.org/reports/tr29/)
- **Not Unicode:** All positions except between `\r` and `\n`.

**[Word mode]**
- Currently, this mode is supported in Unicode only.
- See [Unicode Standard Annex #29](http://unicode.org/reports/tr29/)

---

## 6. Character class

### Operators in Character Class

| Operator | Description |
|----------|-------------|
| `^...` | negative class (lowest precedence) |
| `x-y` | range from x to y |
| `[...]` | set (character class in character class) |
| `..&&..` | intersection (low precedence, only higher than ^) |

> **Note:** If you want to use '[', '-', or ']' as a normal character in character class, you should escape them with '\'.

### POSIX bracket

Format: `[:xxxxx:]`, negate: `[:^xxxxx:]`

#### Not Unicode Case:

| Bracket | Description |
|---------|-------------|
| `[:alnum:]` | alphabet or digit char |
| `[:alpha:]` | alphabet |
| `[:ascii:]` | code value: [0 - 127] |
| `[:blank:]` | `\t`, `\x20` |
| `[:cntrl:]` | control characters |
| `[:digit:]` | 0-9 |
| `[:graph:]` | include all of multibyte encoded characters |
| `[:lower:]` | lowercase letters |
| `[:print:]` | include all of multibyte encoded characters |
| `[:punct:]` | punctuation |
| `[:space:]` | `\t`, `\n`, `\v`, `\f`, `\r`, `\x20` |
| `[:upper:]` | uppercase letters |
| `[:xdigit:]` | 0-9, a-f, A-F |
| `[:word:]` | alphanumeric, "_" and multibyte characters |

#### Unicode Case:

| Bracket | Description |
|---------|-------------|
| `[:alnum:]` | Alphabetic \| Decimal_Number |
| `[:alpha:]` | Alphabetic |
| `[:ascii:]` | U+0000 - U+007F |
| `[:blank:]` | Space_Separator \| U+0009 |
| `[:cntrl:]` | U+0000 - U+001F, U+007F - U+009F |
| `[:digit:]` | Decimal_Number |
| `[:graph:]` | ^White_Space && ^[[:cntrl:]] && ^Unassigned && ^Surrogate |
| `[:lower:]` | Lowercase |
| `[:print:]` | [[:graph:]] \| Space_Separator |
| `[:punct:]` | Punctuation \| Symbol |
| `[:space:]` | White_Space |
| `[:upper:]` | Uppercase |
| `[:xdigit:]` | U+0030 - U+0039 \| U+0041 - U+0046 \| U+0061 - U+0066 (0-9, a-f, A-F) |
| `[:word:]` | Alphabetic \| Mark \| Decimal_Number \| Connector_Punctuation |

---

## 7. Extended groups

### Comments and Options

**Comment:**
```
(?#...)
```

**Option on/off for subexp:**
```
(?imxWDSPy-imxWDSP:subexp)
```

Options:
- `i`: ignore case
- `m`: multi-line (dot (.) also matches newline)
- `x`: extended form
- `W`: ASCII only word (`\w`, `\p{Word}`, `[[:word:]]`) and ASCII only word bound (`\b`)
- `D`: ASCII only digit (`\d`, `\p{Digit}`, `[[:digit:]]`)
- `S`: ASCII only space (`\s`, `\p{Space}`, `[[:space:]]`)
- `P`: ASCII only POSIX properties (includes W,D,S): alnum, alpha, blank, cntrl, digit, graph, lower, print, punct, space, upper, xdigit, word
- `y{?}`: Text Segment mode
  - `y{g}`: Extended Grapheme Cluster mode (default)
  - `y{w}`: Word mode
  - See [Unicode Standard Annex #29](http://unicode.org/reports/tr29/)

**Isolated option:**
```
(?imxWDSPy-imxWDSP)
```
It makes a group to the next ')' or end of the pattern.

> **Note:** `(?i)` option has no effect on word types (`\w`, `\p{Word}`). However, if the word types are used within a character class, it is valid. But, this would only be a concern when word types are used with the `(?W)` option.

**Whole option:**
```
(?CIL)...
(?CIL:...)
```

This option must be placed in a position that affects the entire regular expression.

- `C`: ONIG_OPTION_DONT_CAPTURE_GROUP
- `I`: ONIG_OPTION_IGNORECASE_IS_ASCII
- `L`: ONIG_OPTION_FIND_LONGEST

### Groups

| Pattern | Description |
|---------|-------------|
| `(?:subexp)` | non-capturing group |
| `(subexp)` | capturing group |
| `(?=subexp)` | look-ahead |
| `(?!subexp)` | negative look-ahead |
| `(?<=subexp)` | look-behind |
| `(?<!subexp)` | negative look-behind |
| `(?>subexp)` | atomic group (no backtracks in subexp) |
| `(?<name>subexp)` | define named group |
| `(?'name'subexp)` | define named group (alternative syntax) |

#### Look-behind limitations

- Cannot use Absent stopper `(?~|expr)` and Range clear `(?~|)` operators in look-behind and negative look-behind.
- In look-behind and negative look-behind, support for ignore-case option is limited. Only supports conversion between single characters. (Does not support conversion of multiple characters in Unicode)

#### Named groups

- Each character of the name must be a word character.
- Not only a name but a number is assigned like a capturing group.
- Assigning the same name to two or more subexps is allowed.

### Callouts

#### Callouts of contents

```
(?{...contents...})           callout in progress
(?{...contents...}D)          D is a direction flag char
                              D = 'X': in progress and retraction
                                  '<': in retraction only
                                  '>': in progress only
(?{...contents...}[tag])      tag assigned
(?{...contents...}[tag]D)
```

- Escape characters have no effects in contents.
- contents is not allowed to start with '{'.

```
(?{{{...contents...}}})       n times continuations '}' in contents is allowed in
                              (n+1) times continuations {{{...}}}.
```

Allowed tag string characters: `_` `A-Z` `a-z` `0-9` (first character: `_` `A-Z` `a-z`)

#### Callouts of name

```
(*name)
(*name{args...})              with args
(*name[tag])                  tag assigned
(*name[tag]{args...})
```

Allowed name string characters: `_` `A-Z` `a-z` `0-9` (first character: `_` `A-Z` `a-z`)
Allowed tag string characters: `_` `A-Z` `a-z` `0-9` (first character: `_` `A-Z` `a-z`)

### Absent functions

**Absent repeater** (proposed by Tanaka Akira):
```
(?~absent)
```
This works like `.*` (more precisely `\O*`), but it is limited by the range that does not include the string match with `<absent>`. This is a written abbreviation of `(?~|(?:absent)|\O*)`. `\O*` is used as a repeater.

**Absent expression** (original):
```
(?~|absent|exp)
```
This works like "exp", but it is limited by the range that does not include the string match with `<absent>`.

**Absent stopper** (original):
```
(?~|absent)
```
After passed this operator, string right range is limited at the point that does not include the string match with `<absent>`.

**Range clear:**
```
(?~|)
```
Clear the effects caused by Absent stoppers.

> **Note:** Nested Absent functions are not supported and the behavior is undefined.

### If-then-else

```
(?(condition_exp)then_exp|else_exp)    if-then-else
(?(condition_exp)then_exp)             if-then
```

`condition_exp` can be a backreference number/name or a normal regular expression. When `condition_exp` is a backreference number/name, both `then_exp` and `else_exp` can be omitted. Then it works as a backreference validity checker.

#### Backreference validity checker (original)

```
(?(n)), (?(-n)), (?(+n)), (?(n+level)) ...
(?(<n>)), (?('-n')), (?(<+n>)) ...
(?(<name>)), (?('name')), (?(<name+level>)) ...
```

---

## 8. Backreferences

When we say "backreference a group," it actually means, "re-match the same text matched by the subexp in that group."

### Basic Backreferences

| Pattern | Description |
|---------|-------------|
| `\n` | (n ≥ 1) backreference the nth group in the regexp |
| `\k<n>`, `\k'n'` | (n ≥ 1) backreference the nth group in the regexp |
| `\k<-n>`, `\k'-n'` | (n ≥ 1) backreference the nth group counting backwards from the referring position |
| `\k<+n>`, `\k'+n'` | (n ≥ 1) backreference the nth group counting forwards from the referring position |
| `\k<name>`, `\k'name'` | backreference a group with the specified name |

When backreferencing with a name that is assigned to more than one groups, the last group with the name is checked first, if not matched then the previous one with the name, and so on, until there is a match.

> **Note:** Backreference by number is forbidden if any named group is defined and ONIG_OPTION_CAPTURE_GROUP is not set.

### Backreference with recursion level

(n ≥ 1, level ≥ 0)

```
\k<n+level>  \k'n+level'
\k<n-level>  \k'n-level'

\k<name+level>  \k'name+level'
\k<name-level>  \k'name-level'
```

Destine a group on the recursion level relative to the referring position.

---

## 9. Subexp calls ("Tanaka Akira special")

*Original function*

When we say "call a group," it actually means, "re-execute the subexp in that group."

| Pattern | Description |
|---------|-------------|
| `\g<n>`, `\g'n'` | (n ≥ 1) call the nth group |
| `\g<0>`, `\g'0'` | call zero (call the total regexp) |
| `\g<-n>`, `\g'-n'` | (n ≥ 1) call the nth group counting backwards from the calling position |
| `\g<+n>`, `\g'+n'` | (n ≥ 1) call the nth group counting forwards from the calling position |
| `\g<name>`, `\g'name'` | call the group with the specified name |

### Restrictions

- Left-most recursive calls are not allowed.

- Calls with a name that is assigned to more than one groups are not allowed.

- Call by number is forbidden if any named group is defined and ONIG_OPTION_CAPTURE_GROUP is not set.

- The option status of the called group is always effective.

---

## 10. Captured group

Behavior of an unnamed group `(...)` changes with the following conditions. (But named group is not changed.)

### Case 1: `/.../` (named group is not used, no option)

`(...)` is treated as a capturing group.

### Case 2: `/.../g` (named group is not used, 'g' option)

`(...)` is treated as a non-capturing group `(?:...)`.

### Case 3: `/..(?<name>..)../` (named group is used, no option)

`(...)` is treated as a non-capturing group. numbered-backref/call is not allowed.

### Case 4: `/..(?<name>..)../G` (named group is used, 'G' option)

`(...)` is treated as a capturing group. numbered-backref/call is allowed.

**where:**
- `g`: ONIG_OPTION_DONT_CAPTURE_GROUP
- `G`: ONIG_OPTION_CAPTURE_GROUP

('g' and 'G' options are argued in ruby-dev ML)

---

## Appendix

### A-1. Syntax-dependent options

**ONIG_SYNTAX_ONIGURUMA:**
- `(?m)`: dot (.) also matches newline

**ONIG_SYNTAX_PERL and ONIG_SYNTAX_JAVA:**
- `(?s)`: dot (.) also matches newline
- `(?m)`: ^ matches after newline, $ matches before newline

### A-2. Original extensions

- hexadecimal digit char type: `\h`, `\H`
- true anychar: `\O`
- text segment boundary: `\y`, `\Y`
- backreference validity checker: `(?(...))` 
- named group: `(?<name>...)`, `(?'name'...)`
- named backref: `\k<name>`
- subexp call: `\g<name>`, `\g<group-num>`
- absent expression: `(?~|...|...)`
- absent stopper: `(?~|...)`

### A-3. Missing features compared with perl 5.8.0

- `\N{name}`
- `\l`, `\u`, `\L`, `\U`, `\C`
- `(??{code})`

> **Note:** `\Q...\E` is effective on ONIG_SYNTAX_PERL and ONIG_SYNTAX_JAVA.

### A-4. Differences with Japanized GNU regex(version 0.12) of Ruby 1.8

- add character property (`\p{property}`, `\P{property}`)
- add hexadecimal digit char type (`\h`, `\H`)
- add look-behind: `(?<=fixed-width-pattern)`, `(?<!fixed-width-pattern)`
- add possessive quantifier: `?+`, `*+`, `++`
- add operations in character class: `[]`, `&&` ('[' must be escaped as an usual char in character class.)
- add named group and subexp call.
- octal or hexadecimal number sequence can be treated as a multibyte code char in character class if multibyte encoding is specified. (ex. `[\xa1\xa2]`, `[\xa1\xa7-\xa4\xa1]`)
- allow the range of single byte char and multibyte char in character class. (ex. `/[a-<<any EUC-JP character>>]/` in EUC-JP encoding.)
- effect range of isolated option is to next ')'. (ex. `(?:(?i)a|b)` is interpreted as `(?:(?i:a|b))`, not `(?:(?i:a)|b)`.)
- isolated option is not transparent to previous pattern. (ex. `a(?i)*` is a syntax error pattern.)
- allowed unpaired left brace as a normal character. (ex. `/{/`, `/({)/`, `/a{2,3/` etc...)
- negative POSIX bracket `[:^xxxx:]` is supported.
- POSIX bracket `[:ascii:]` is added.
- repeat of look-ahead is not allowed. (ex. `/(?=a)*/`, `/(?!b){5}/`)
- Ignore case option is effective to escape sequence. (ex. `/\x61/i =~ "A"`)
- In the range quantifier, the number of the minimum is optional. (`/a{,n}/` == `/a{0,n}/`. The omission of both minimum and maximum values is not allowed: `/a{,}/`)
- `/{n}?/` is not a reluctant quantifier. (`/a{n}?/` == `/(?:a{n})?/`)
- invalid back reference is checked and raises error. (`/\1/`, `/(a)\2/`)
- Zero-width match in an infinite loop stops the repeat, then changes of the capture group status are checked as stop condition. (`/(?:()|())*\1\2/ =~ ""`, `/(?:\1a|())*/ =~ "a"`)

---

## 11. Substitution References

When using `regex_replace`, the following references can be used in the replacement string to insert matched content:

### Standard References

| Reference | Description |
|-----------|-------------|
| `$n` | Numbered reference. Inserts the string matched by the nth capture group (n ≥ 1). |
| `$&` | Entire matched string. Inserts the full match. |
| `` $` `` | Prefix. Inserts the text before the match. |
| `$'` | Suffix. Inserts the text after the match. |
| `$$` | Literal dollar sign. Inserts a single `$`. |
| `${name}` | Named reference. Alternative syntax for named group insertion. |

### Oniguruma-specific References

The following references are available only when the `oniguruma` flag is specified:

| Reference | Description |
|-----------|-------------|
| `${n}` | Safe numbered reference. Allows digits immediately after (e.g., `${1}0` inserts group 1 followed by '0'). |
| `\k<name>` | Named reference. Inserts the string matched by the named group. |
| `\n` | Numbered reference. Inserts the string matched by the nth capture group (n ≥ 1). |
| `\0` | Entire matched string. Equivalent to `$&`. |
| `\\` | Literal backslash. Inserts a single `\`. |

## Examples

```cpp
// Standard $1, $2 references
regex re(R"((\w+):(\w+))");
string result = regex_replace("key:value", re, "$2=$1");
// result: "value=key"

// Entire match with $&
regex re2(R"(\w+)");
string result2 = regex_replace("hello world", re2, "[$&]");
// result2: "[hello] [world]"

// Named reference with Oniguruma flag
regex re3(R"((?<word>\w+))", regex::oniguruma);
string result3 = regex_replace("hello", re3, "${word}!");
// result3: "hello!"
```

---

## 12. format_literal Function

The `format_literal` function processes C++ escape sequences in a string literal, converting them to their actual character values. This is useful when working with regex patterns from configuration files or user input where escape sequences should be interpreted as in C++ string literals.

### Supported Escape Sequences

| Escape | Description | Value |
|--------|-------------|-------|
| `\\` | backslash | `\` |
| `\n` | newline (LF) | `0x0A` |
| `\r` | carriage return (CR) | `0x0D` |
| `\t` | horizontal tab | `0x09` |
| `\v` | vertical tab | `0x0B` |
| `\f` | form feed | `0x0C` |
| `\a` | alert (bell) | `0x07` |
| `\b` | backspace | `0x08` |
| `\0` | null character | `0x00` |
| `\xHH` | hexadecimal escape (2 hex digits) | encoded byte value |
| `\uHHHH` | Unicode escape (4 hex digits) | UTF-8 encoded |
| `\UHHHHHHHH` | Unicode escape (8 hex digits) | UTF-8 encoded |
| `\ooo` | octal escape (1-3 octal digits) | encoded byte value |

Unknown escape sequences (e.g., `\q`) are passed through unchanged.

### Usage

```cpp
#include "onigpp.h"
#include <iostream>

int main() {
    using namespace onigpp;
    
    // Process escape sequences from a configuration string
    std::string pattern = format_literal("\\d+\\.\\d+");
    // pattern: "\d+\.\d+" (actual backslash characters)
    
    // Process newlines and tabs
    std::string text = format_literal("line1\\nline2\\ttabbed");
    // text: "line1\nline2\ttabbed" (actual newline and tab)
    
    // Unicode characters
    std::string euro = format_literal("Price: \\u20AC100");
    // euro: "Price: €100"
    
    // Use with regex - pattern from config file
    regex re(format_literal("\\\\w+"));  // matches word characters
    smatch m;
    if (regex_search("hello world", m, re)) {
        std::cout << m[0] << std::endl;  // prints "hello"
    }
    
    return 0;
}
```

### Function Signatures

```cpp
namespace onigpp {
    template <class CharT>
    std::basic_string<CharT> format_literal(const std::basic_string<CharT>& str);
    
    template <class CharT>
    std::basic_string<CharT> format_literal(const CharT* str);
    
    template <class CharT>
    std::basic_string<CharT> format_literal(const CharT* str, size_type len);
}
```

> **Note:** Raw string literals (e.g., `R"(pattern)"`) do not need processing as they already preserve the literal text without escape interpretation.

---

## See also

- [Unicode Standard Annex #29 - Unicode Text Segmentation](http://unicode.org/reports/tr29/)
