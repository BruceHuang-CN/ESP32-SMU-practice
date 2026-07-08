# Control Web Module Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build the first project module: global state, alarm priority, LED/buzzer control, serial commands, and Wi-Fi web dashboard with command buttons.

**Architecture:** Keep sensor/alarm state in a pure C module that can be host-tested. ESP-IDF-specific code is limited to web server, GPIO alarm output, and serial command task.

**Tech Stack:** ESP-IDF C, FreeRTOS, ESP Wi-Fi AP, `esp_http_server`, GPIO driver, host-side GCC test for pure logic.

---

### Task 1: Core State And Commands

**Files:**
- Create: `main/system_state.h`
- Create: `main/system_state.c`
- Create: `tests/test_system_state.c`

- [ ] Write tests for alarm priority and commands.
- [ ] Run host test and confirm it fails before implementation.
- [ ] Implement state initialization, alarm priority, command parsing, and status formatting.
- [ ] Run host test and confirm it passes.

### Task 2: Web Dashboard And HTTP Commands

**Files:**
- Create: `main/web_server.h`
- Create: `main/web_server.c`

- [ ] Add Wi-Fi AP startup using SSID `一艘船` and password `123456789`.
- [ ] Serve `/` with the dashboard HTML.
- [ ] Serve `/data` with JSON generated from `system_state_t`.
- [ ] Serve `/cmd?name=ALARM_OFF`, `/cmd?name=ALARM_ON`, and `/cmd?name=STATUS`.

### Task 3: LED And Buzzer Alarm Output

**Files:**
- Create: `main/alarm_control.h`
- Create: `main/alarm_control.c`

- [ ] Configure GPIO13 LED and GPIO14 buzzer as active-high outputs.
- [ ] Add a FreeRTOS task that updates output patterns according to current alarm priority.
- [ ] Respect `alarm_output_enabled`; web/serial `ALARM_OFF` silences outputs without hiding alarm state.

### Task 4: Main Integration And Serial Commands

**Files:**
- Modify: `main/main.c`
- Modify: `main/CMakeLists.txt`

- [ ] Initialize shared state.
- [ ] Start alarm output and web server.
- [ ] Add serial command task supporting `STATUS`, `ALARM_OFF`, `ALARM_ON`, and `HELP`.
- [ ] Print startup information and AP address.

### Task 5: Verification

**Files:**
- No new files.

- [ ] Run host test with GCC.
- [ ] Run ESP-IDF build with `idf.py build`.
- [ ] Report any environment/toolchain issue clearly.

### Self-Review

- Spec coverage: covers first module, Wi-Fi dashboard, web buttons, serial commands, alarm priority, LED/buzzer policy.
- Placeholder scan: no implementation placeholders are required for this first module; sensor modules are intentionally represented by state fields until later modules are added.
- Type consistency: `system_state_t` is the single shared state type used by web, alarm, and serial code.
