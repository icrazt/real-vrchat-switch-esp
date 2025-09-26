# Lighting Control Implementation

This document describes the changes that implement the button-driven lighting control logic.

## Overview
- Button handling and lighting control now run in separate FreeRTOS tasks using a producer/consumer pattern.
- The main button is processed by the OneButton library to publish high-level events into a queue.
- The RGB task consumes those events to manage brightness (bright/dim) and glow modes (solid, breathing, rainbow, theatre chase, theatre chase rainbow) without blocking animation updates.

## Button Task (TaskButton)
- Configures OneButton with handlers for single clicks and long presses (> 1000 ms).
- Each callback posts a ButtonEvent (single click or long press) into the shared FreeRTOS queue.
- Runs continuously with a short delay to debounce via the OneButton tick() API.

## RGB Task (TaskRGB)
- Maintains the current brightness mode and glow mode alongside requested values from the queue.
- Responds to events:
  - Single click toggles bright (high brightness) and dim (low brightness) modes.
  - Long press steps through the glow-mode list in a round-robin fashion.
- For solid and breathing modes, the task drives the strip directly; for other animations it defers to StrandtestController::update() each loop iteration.
- Includes a non-blocking breathing effect that adjusts brightness in small steps on a timer so it coexists with other FreeRTOS tasks.

## Brightness Synchronisation
- Helper utilities compute brightness bounds for each mode.
- When modes change, the task updates the underlying Adafruit_NeoPixel strip and StrandtestController brightness so all effects respect the current dim/bright setting.


## Initialization (setup)
- Sets GPIO modes for the LED and RGB enable pin, powers the strip, and starts serial logging.
- Creates the button-event queue and launches the two tasks with their respective stack sizes and priorities.
- The loop function no longer performs work - tasks handle runtime behaviour while the main loop sleeps.

## State Persistence
- Lighting preferences use the ESP32 Preferences (NVS) API under the lighting namespace.
- The last applied brightness and glow modes are saved whenever they change and reloaded when the RGB task starts, providing consistent behaviour after power loss.

Endurance Estimate

ESP32 NVS stores our brightness + glow keys in flash pages (4 KB each). Each update consumes one entry (~32 B), and a page erase is only triggered after ~128 updates, so we see roughly 12.8 M writes per page (100 k erase endurance × 128 entries).
Both keys live in the same namespace, so two writes per state change halve that budget to ~6.4 M full state saves before the page wears out; wear-levelling spreads this across at least two pages, so practical endurance is still well above 10 M cycles.
If a user changes modes 500 times per day, that’s ≈20 k writes/year, implying 300+ years before the theoretical flash limit—far beyond the device’s expected lifetime.
Heavy stress testing (thousands of toggles per hour) could push toward the limit sooner but still leaves several decades of margin.
Mitigation Ideas

Add debounce logic for persistence (e.g., save only after a short idle period) to reduce redundant writes.
Expose a “factory reset” or namespace-clear option to recover flash if corruption ever occurs.
