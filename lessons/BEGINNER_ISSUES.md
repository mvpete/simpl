# SIMPL Beginner Issues

This page turns existing TODOs and learning gaps into beginner-friendly contribution tasks.

## Good First Tasks

### 1) Complete missing tokenizer tests
- **File:** `simpl.test/simpl.tokenizer.test.cpp`
- **Why it matters:** strengthens confidence in core token handling.
- **Suggested scope:** implement and run tests for:
  - `TestTokenParenthesis`
  - `TestTokenSqBracket`
  - `TestTokenBracket`
  - `TestTokenComma`

### 2) Improve token type assertion output
- **File:** `simpl.test/simpl.tokenizer.test.cpp`
- **TODO reference:** “Better formatting w/ switch” in `ToString<simpl::token_types>`.
- **Why it matters:** test failures become easier to debug.
- **Suggested scope:** replace numeric output with friendly enum names.

### 3) Refactor value constructor overloads
- **File:** `include/simpl/value.h`
- **TODO reference:** “Use variadics for this, and just use the value_t”.
- **Why it matters:** cleaner API and less duplication.
- **Suggested scope:** add tests first, then small safe refactor.

### 4) VM execution context cleanup
- **File:** `include/simpl/vm_execution_context.h`
- **TODO references:** optimization/refactor notes around initializer handling and apply patterns.
- **Why it matters:** reduces complexity in core execution logic.
- **Suggested scope:** pick one TODO at a time, with focused tests.

## Contribution Format

For each task:
1. Keep PRs small and focused on one issue.
2. Include tests or manual validation steps.
3. Include a short “Before/After” note in your PR description.

## Where to Start

If this is your first contribution, start with **Task 1**.  
It has low risk, clear acceptance criteria, and immediate learning value.
