# Debugging SIMPL Programs

This guide helps you quickly debug common SIMPL errors while working through the lessons.

## 1) Parse/Token Errors

### Symptom
You see errors like:
- `failed to parse`
- unexpected token messages

### Common causes
- Missing semicolon `;`
- Unbalanced `()`, `[]`, or `{}`
- Unterminated string literal
- Typo in a directive or keyword

### Quick checks
1. Verify statement terminators (`;`) where expected.
2. Count opening and closing delimiters.
3. Reduce to a minimal failing snippet.

## 2) “Function not found” / Missing features

### Symptom
Calling `print`, `println`, array helpers, or file/gui helpers fails.

### Cause
Required library was not imported.

### Fix
Add an import near the top of your script:
```simpl
@import io
@import array
```

## 3) Type mismatch at runtime

### Symptom
Runtime exceptions when passing values to bound C++ functions or multi-methods.

### Cause
The value type in script does not match expected function signature.

### Fix
1. Print intermediate values.
2. Verify method overload conditions (e.g., `is string`, `is number`).
3. Narrow to a single failing call path.

## 4) Debugging workflow for lessons

1. Reproduce the failure with the smallest possible script.
2. Add temporary `println(...)` checkpoints.
3. Compare behavior with a working example in `examples/`.
4. If changing C++ internals, add or update a targeted test in `simpl.test/`.

## 5) Helpful REPL tips

- Use `:help` to list built-in REPL commands.
- Use `:examples` for quick script snippets.
- Use `:quit` (or `exit();`) to leave the REPL.

## 6) Escalation path

If you are still blocked:
1. Open an issue with your script snippet and exact error output.
2. Include environment details (OS, toolchain, Visual Studio version if applicable).
3. Link to the lesson where the issue occurred.
