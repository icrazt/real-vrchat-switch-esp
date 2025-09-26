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
