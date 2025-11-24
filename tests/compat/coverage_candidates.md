# Compatibility Test Coverage Candidates

This document lists priority test cases for systematic coverage of compatibility differences between `onigpp` and `std::regex`. Each candidate includes a unique ID, pattern, flags, input, operation type, encoding hints, and rationale for why this case is important for detecting behavioral differences.

## Purpose

These candidates serve as a roadmap for expanding `patterns.json` and automated compatibility testing. Each case targets specific functionality where implementations may differ.

## Test Case Format

Each entry includes:
- **id**: Unique identifier
- **pattern**: Regular expression pattern
- **flags**: Syntax flags (ECMAScript, icase, nosubs, etc.)
- **input**: Test input string
- **operation**: match, search, replace, iterator, or token_iterator
- **replace_template**: (for replace operations) Replacement pattern
- **encoding_hint**: UTF-8, SJIS, UTF-16LE, etc. (if relevant)
- **rationale**: Why this case matters and expected differences

---

## Basic Matching and Anchors

### 1. simple_exact_match
- **pattern**: `^hello$`
- **flags**: ECMAScript
- **input**: `hello`
- **operation**: match
- **encoding_hint**: UTF-8
- **rationale**: Basic exact match with anchors. Verifies fundamental match behavior is consistent.

### 2. partial_match_anchor_failure
- **pattern**: `^hello$`
- **flags**: ECMAScript
- **input**: `hello world`
- **operation**: match
- **encoding_hint**: UTF-8
- **rationale**: Anchored pattern should fail on partial match. Tests anchor semantics at string boundaries.

### 3. basic_search
- **pattern**: `world`
- **flags**: ECMAScript
- **input**: `hello world`
- **operation**: search
- **encoding_hint**: UTF-8
- **rationale**: Simple substring search. Verifies basic search vs match distinction.

---

## Capture Groups and Backreferences

### 4. capture_groups_basic
- **pattern**: `(\w+):(\w+)`
- **flags**: ECMAScript
- **input**: `a:1`
- **operation**: search
- **encoding_hint**: UTF-8
- **rationale**: Tests capture group extraction, counting, and content. Position/length accuracy matters.

### 5. backreference_simple
- **pattern**: `(\w+)\s+\1`
- **flags**: ECMAScript
- **input**: `hi hi`
- **operation**: search
- **encoding_hint**: UTF-8
- **rationale**: Backreference semantics can differ between engines (case sensitivity, capture scope).

---

## Zero-Width Assertions

### 6. zero_width_boundary
- **pattern**: `\bword\b`
- **flags**: ECMAScript
- **input**: `word`
- **operation**: search
- **encoding_hint**: UTF-8
- **rationale**: Word boundary behavior differs with Unicode and multibyte characters.

### 11. lookahead_positive
- **pattern**: `a(?=b)`
- **flags**: ECMAScript
- **input**: `ab`
- **operation**: search
- **encoding_hint**: UTF-8
- **rationale**: Positive lookahead availability and behavior. Some engines have limited support.

### 12. lookbehind_positive
- **pattern**: `(?<=a)b`
- **flags**: ECMAScript
- **input**: `ab`
- **operation**: search
- **encoding_hint**: UTF-8
- **rationale**: Positive lookbehind is not universally supported in ECMAScript mode. Tests implementation limits.

### 13. negative_lookaround
- **pattern**: `a(?!b)`
- **flags**: ECMAScript
- **input**: `ac`
- **operation**: search
- **encoding_hint**: UTF-8
- **rationale**: Negative lookahead/lookbehind handling varies. Important for complex patterns.

### 35. lookaround_at_boundaries
- **pattern**: `(?<=^)hello`
- **flags**: ECMAScript
- **input**: `hello`
- **operation**: search
- **encoding_hint**: UTF-8
- **rationale**: Lookaround at string boundaries can expose edge-case differences.

---

## Replacement Operations

### 7. replace_dollar_refs
- **pattern**: `(a)(b)`
- **flags**: ECMAScript
- **input**: `abx`
- **operation**: replace
- **replace_template**: `$2$1`
- **encoding_hint**: UTF-8
- **rationale**: Tests $1, $2, etc. substitution in replacement. Numbering and escaping can differ.

### 27. regex_replace_specials
- **pattern**: `(\w+)`
- **flags**: ECMAScript
- **input**: `hello world`
- **operation**: replace
- **replace_template**: `$&-$\`-$'`
- **encoding_hint**: UTF-8
- **rationale**: Tests $&, $\`, $' (matched text, prefix, suffix) support. Not all engines support these.

### 28. format_flags_behavior
- **pattern**: `\w+`
- **flags**: ECMAScript
- **input**: `hello world`
- **operation**: replace
- **replace_template**: `X`
- **encoding_hint**: UTF-8
- **rationale**: Tests format_first_only, format_no_copy flags. Behavior may differ in how many replacements occur or whether unmatched text is copied.

---

## Flags and Options

### 8. nosubs_flag_behavior
- **pattern**: `(x)`
- **flags**: nosubs
- **input**: `x`
- **operation**: search
- **encoding_hint**: UTF-8
- **rationale**: nosubs flag should prevent submatch storage. Tests whether size==1 and no captures are stored.

### 9. icase_flag_behavior
- **pattern**: `abc`
- **flags**: icase
- **input**: `AbC`
- **operation**: search
- **encoding_hint**: UTF-8
- **rationale**: Case-insensitive matching with ASCII. Ensure consistent behavior across implementations.

### 24. locale_sensitive_collation
- **pattern**: `[a-z]+`
- **flags**: collate
- **input**: `ABC`
- **operation**: search
- **encoding_hint**: UTF-8
- **rationale**: collate flag enables locale-sensitive collation. Behavior may vary by locale/platform.

### 34. anchors_multiline
- **pattern**: `^line`
- **flags**: multiline
- **input**: `first\nline`
- **operation**: search
- **encoding_hint**: UTF-8
- **rationale**: Multiline mode changes ^ and $ to match line boundaries. Newline handling can differ.

---

## Encoding and Multibyte

### 10. utf8_multibyte_char
- **pattern**: `あ`
- **flags**: ECMAScript
- **input**: `あいう`
- **operation**: search
- **encoding_hint**: UTF-8
- **rationale**: UTF-8 multibyte character matching. Position/length calculations must be byte-accurate.

### 14. sjis_byte_sequence
- **pattern**: `あ` (SJIS encoded)
- **flags**: ECMAScript
- **input**: `あい` (SJIS encoded)
- **operation**: search
- **encoding_hint**: SJIS
- **rationale**: SJIS encoding character matching. Tests encoding-specific handling and multibyte character support in SJIS.

### 15. utf8_boundary_detection
- **pattern**: `\b\w+\b`
- **flags**: ECMAScript
- **input**: `Hello 世界 test`
- **operation**: search
- **encoding_hint**: UTF-8
- **rationale**: Word boundaries with mixed ASCII and multibyte UTF-8 characters. Tests boundary detection across character encodings.

### 16. empty_pattern_match
- **pattern**: ``
- **flags**: ECMAScript
- **input**: `test`
- **operation**: search
- **encoding_hint**: UTF-8
- **rationale**: Empty pattern behavior. Some engines match empty string at every position, others may throw error.

### 17. alternation_priority
- **pattern**: `a|ab`
- **flags**: ECMAScript
- **input**: `ab`
- **operation**: search
- **encoding_hint**: UTF-8
- **rationale**: Tests alternation priority and greedy vs left-to-right matching. Different engines may prioritize differently.

### 22. surrogate_pairs_utf16
- **pattern**: `𝐀`
- **flags**: ECMAScript
- **input**: `𝐀𝐁`
- **operation**: search
- **encoding_hint**: UTF-16LE
- **rationale**: UTF-16 surrogate pair handling. Oniguruma may differ from std::regex in code unit vs code point handling.

### 30. encoding_mismatch_handling
- **pattern**: `test`
- **flags**: ECMAScript
- **input**: `test`
- **operation**: search
- **encoding_hint**: SJIS (pattern in UTF-8)
- **rationale**: When pattern encoding and input encoding mismatch, behavior should be predictable or error should be thrown.

### 33. multistring_match
- **pattern**: `(あ)(い)`
- **flags**: ECMAScript
- **input**: `あいう`
- **operation**: search
- **encoding_hint**: UTF-8
- **rationale**: Multibyte mixed-content matching with captures. Position/length must account for byte vs character units.

---

## Quantifiers

### 18. non_greedy_quantifiers
- **pattern**: `a.*?b`
- **flags**: ECMAScript
- **input**: `aXXbYYb`
- **operation**: search
- **encoding_hint**: UTF-8
- **rationale**: Non-greedy quantifiers should match minimally. Verify correct minimal match behavior.

### 19. possessive_quantifiers
- **pattern**: `a++b`
- **flags**: ECMAScript
- **input**: `aab`
- **operation**: search
- **encoding_hint**: UTF-8
- **rationale**: Possessive quantifiers (a++) prevent backtracking. Not standard ECMAScript; may cause parse error or be unsupported.

---

## Unicode Properties

### 20. unicode_property
- **pattern**: `\p{Han}`
- **flags**: ECMAScript
- **input**: `漢字`
- **operation**: search
- **encoding_hint**: UTF-8
- **rationale**: Unicode property classes (\p{Han}, \p{L}) may not be supported in all ECMAScript modes. Tests availability.

### 21. grapheme_cluster
- **pattern**: `e\u0301`
- **flags**: ECMAScript
- **input**: `café` (é as e + combining acute)
- **operation**: search
- **encoding_hint**: UTF-8
- **rationale**: Combining characters and grapheme clusters. Matching may differ when combining marks are involved.

---

## POSIX and Character Classes

### 23. posix_character_classes
- **pattern**: `[[:alpha:]]+`
- **flags**: ECMAScript
- **input**: `hello123`
- **operation**: search
- **encoding_hint**: UTF-8
- **rationale**: POSIX character classes like [:alpha:] may not be supported in ECMAScript mode. Tests compatibility.

---

## Iterators

### 25. regex_iterator_basic
- **pattern**: `\w+`
- **flags**: ECMAScript
- **input**: `a b c`
- **operation**: iterator
- **encoding_hint**: UTF-8
- **rationale**: regex_iterator iteration and zero-width advancement. Edge cases in empty matches or overlapping matches.

### 26. regex_token_iterator_subs
- **pattern**: `(\w+)`
- **flags**: ECMAScript
- **input**: `a b`
- **operation**: token_iterator
- **encoding_hint**: UTF-8
- **rationale**: token_iterator with -1 (unmatched parts) or specific capture index. Behavior can differ in which tokens are yielded.

### 29. iterator_non_random_access
- **pattern**: `\w+`
- **flags**: ECMAScript
- **input**: `a b c` (using std::list)
- **operation**: search
- **encoding_hint**: UTF-8
- **rationale**: Non-random-access iterators (e.g., std::list::const_iterator) must still support position() and length(). Tests iterator category compatibility.

---

## Error Handling and Edge Cases

### 31. compiled_regex_copy_move
- **pattern**: `test`
- **flags**: ECMAScript
- **input**: `test`
- **operation**: search
- **encoding_hint**: UTF-8
- **rationale**: Copy/move semantics of compiled regex objects. Verify no double-free or invalid state after copy/move.

### 32. exception_type_and_message
- **pattern**: `(`
- **flags**: ECMAScript
- **input**: `test`
- **operation**: match
- **encoding_hint**: UTF-8
- **rationale**: Invalid pattern should throw regex_error. Compare error code and what() message for consistency.

### 36. large_pattern_complexity
- **pattern**: `(a+)+b` (applied to large input)
- **flags**: ECMAScript
- **input**: `aaaa...aaa` (1000+ 'a's, no 'b')
- **operation**: search
- **encoding_hint**: UTF-8
- **rationale**: Catastrophic backtracking or complexity limit. Should throw error_complexity or timeout. Tests resource limit handling.

---

## Summary

Total: **36 candidates** (IDs 1-36) covering:
- Basic matching and anchors (3)
- Capture groups and backreferences (2)
- Zero-width assertions (5)
- Replacement operations (3)
- Flags and options (4)
- Encoding and multibyte (8)
- Quantifiers (2)
- Unicode properties (2)
- POSIX and character classes (1)
- Iterators (3)
- Error handling and edge cases (3)

## Next Steps

1. Select test cases to implement in `patterns.json`
2. Update `compat_test.cpp` if new operation types are needed (iterator, token_iterator)
3. Add CI integration to run these tests automatically
4. Document discovered differences and compatibility issues

## Notes

- For non-UTF-8 encodings (SJIS, UTF-16), test data should be provided in the appropriate encoding
- Some tests may require extending compat_test.cpp to support iterator operations
- Not all differences indicate bugs—some are implementation-specific design choices
- Implementation of encoding-specific tests may require additional infrastructure support
