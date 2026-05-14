# pig-babysitter

Zephyr RTOS migration baseline for the pig-babysitter MCU project.

## Scope (current iteration)

- Target RTOS: **Zephyr**
- Goal: **minimal compilable/runnable baseline** first, then incremental feature migration

## Current structure inventory

- Existing content before migration: a minimal placeholder README only
- Boot/runtime entry: now provided by Zephyr app `src/main.c`
- Peripheral dependencies: currently abstracted behind HAL stubs
- Build flow: Zephyr `west build` based flow

## Added Zephyr baseline

- `CMakeLists.txt`: Zephyr app entry
- `Kconfig` + `prj.conf`: app config and defaults
- `boards/native_sim.overlay`: DTS overlay placeholder
- `include/pig_babysitter/*`: migration layer headers
- `src/*`: minimal layered runtime

## Layered migration path

1. HAL / Drivers
   - Replace functions in `src/hal_stubs.c` with real board/peripheral support
2. Business logic
   - Move existing logic into `src/app_core.c` (or split into modules)
3. Communication / Storage
   - Implement real protocol + storage backends in `src/comms_stubs.c`

## Validation steps

Run after each migration step:

```bash
west build -b native_sim .
west build -t run
```

Then add board-specific verification for timing, peripheral behavior, and memory usage.

## Convergence checklist before mainline switch

- Functional behavior matches legacy project
- Task/loop timing remains within expected limits
- Flash/RAM usage is acceptable for target board
- Migration notes are complete for remaining gaps
