# GUI Library Status

This document describes the current state of SIMPL's `gui` library.

## Current Support

The current GUI library is Win32-based and supports:

- `window` controls
- `button` controls
- `text` (static label) controls
- `edit` (editable text input) controls
- Basic window/control operations:
  - `create_wnd`, `create_btn`, `create_text`, `create_edit`
  - `set_text`, `get_text`
  - `set_pos`, `get_pos`
  - `show`, `show_async`, `poll`, `run`, `close`, `quit`
- Basic event hooks:
  - `on_click` for `button`, `text`, and `edit`
  - `on_focus` for `button`, `text`, and `edit`
  - `on_blur` for `button`, `text`, and `edit`
  - `on_change` for `edit`

## Previously Partial Areas and Current Gaps

## Stability

- Fixed: 64-bit unsafe window subclass pointer usage has been replaced with `SetWindowLongPtrA`/`GetWindowLongPtrA`.
- Remaining: message and callback behavior still relies on Win32 control message semantics and should continue to be validated with integration-style tests.

## API Surface

- Improved: editable text input and lifecycle helpers were added.
- Remaining: no menus, list views, dialogs, or richer layout/container APIs.

## Events

- Improved: click/focus/blur/change hooks are available for core controls.
- Remaining: no high-level event payloads (for example, key codes or mouse coordinates) and no generalized event dispatch abstraction.

## Portability

- By design, current GUI support is Win32-only.

## Validation Coverage

- Improved: GUI registration and callback wiring paths now have coverage in `simpl.test/simpl.engine.test.cpp`.
- Remaining: no end-to-end UI automation tests.

## Roadmap

1. Stabilize and expand event behavior tests.
2. Add more essential controls (checkbox, list, combo, and multiline edit).
3. Add richer lifecycle and state APIs (visibility, enable/disable, parent/child introspection).
4. Add higher-level layout helpers.
5. Add optional integration test harness for GUI event flows.
