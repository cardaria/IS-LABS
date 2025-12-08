# Lab 3 Interrupt Walkthrough (beginner-friendly)

This guide explains, step by step, what the code in `labmain.c` and `boot.S` does, why each piece exists, and how the interrupts connect from power-on until the program runs.

## Big picture
- We keep time with a hardware **timer** (IRQ 16) that ticks every 0.1 s.
- A chosen **switch** (IRQ 17) can add `TIME_INCREMENT` seconds whenever you flip it.
- Optionally, the **push-button** (IRQ 18) can add the same increment so you can use both inputs.
- All three sources are wired through `boot.S` to the shared C interrupt handler `handle_interrupt`.

## Configuration knobs in `labmain.c`
- `SWITCH_NUMBER`: pick which physical switch (2 or 3) should trigger the increment. The code turns that into a bit position.
- `TIME_INCREMENT`: how many seconds to add per switch or button interrupt (matches the Lab 3 surprise variants).
- `SWITCH_DEBOUNCE_MS`: tiny delay (10 ms) used after a switch/button edge so bouncing plastic does not double-trigger.
- `ENABLE_BUTTON_INCREMENT`: set to 1 if you want the button to increment time; set to 0 to ignore button presses.

## Boot and setup flow (ordered)
1. **`_start` in `boot.S`:**
   - Clears the machine status so no interrupts can fire during setup.
   - Sets up stack and global pointers.
   - Prints a short hello string and then jumps into `main` (C code).
2. **`main` calls `labinit`:**
   - **Timer setup:** load a 0.1 s period into the Avalon timer registers and turn on its interrupt enable bit.
   - **Switch setup:** configure pins as inputs, clear any old edge flags, and unmask only the chosen switch bit so IRQ 17 fires when that switch changes.
   - **Button setup:** configure as input, clear edge flags, and unmask bit 0 if `ENABLE_BUTTON_INCREMENT` is turned on.
   - **Displays:** immediately draw the starting time so you see something on the 7-seg.
   - **Interrupt enable:** call `enable_interrupt()` to point `mtvec` at `_isr_handler`, allow IRQ 16/17/18 in `mie`, and set the global `mstatus.MIE` bit. After this line, interrupts are live.
3. **`main` loop:** does nothing but print prime numbers forever. Timekeeping is 100% driven by interrupts.

## How the interrupt path works
1. **Hardware fires:** the timer, switch, or button asserts its IRQ line (16/17/18).
2. **`_isr_handler` (assembly) runs:**
   - Saves caller registers on the stack.
   - Reads `mcause` to see why we trapped.
   - If it was an external interrupt, it strips off the top bit and passes the IRQ number (16/17/18) into `handle_interrupt` in C.
   - Restores registers and executes `mret` to return when `handle_interrupt` finishes.
3. **`handle_interrupt` (C) chooses a branch:**
   - **Timer (16):** check the timer status bit, clear it, count up to ten ticks, then add one second with `advance_time_seconds(1)` and redraw the displays. No delays here, so ticks stay steady.
   - **Switch (17):** if the configured switch bit shows an edge:
     1. Mask switch IRQs so bouncing cannot immediately re-enter.
     2. Clear the captured edge and save the latest switch state.
     3. Add `TIME_INCREMENT` seconds.
     4. Wait `SWITCH_DEBOUNCE_MS` so the contact settles.
     5. Clear any bounce edges and unmask the chosen switch bit.
   - **Button (18):** if `ENABLE_BUTTON_INCREMENT` is on and bit 0 edged high:
     1. Mask the button IRQ.
     2. Clear the edge.
     3. Add `TIME_INCREMENT` seconds (same helper as the switch).
     4. Wait the short debounce delay.
     5. Clear any bounce and unmask the button line.
     Otherwise, we simply clear the edge to keep it quiet.

## Data and display flow inside `labmain.c`
- **Time storage:** `mytime` keeps seconds and minutes in BCD; `hours` holds 0–23 separately. `advance_time_seconds` loops over `tick(&mytime)`, handles the wrap from 59:59 back to 00:00, bumps `hours`, and then refreshes the digits.
- **Display helper:** `update_displays` slices each digit (sec/min/hour) and calls `set_displays`. That helper maps a number 0–9 to the 7-seg pattern and writes it to the right offset in the display array.
- **State variables:** `timeout_counter` counts ten timer ticks before adding one second; `previous_switch_state` lets us keep an eye on the last switch reading.

## How to enable button-based increments (both inputs at once)
1. In `labmain.c`, leave `ENABLE_BUTTON_INCREMENT` set to `1` (default in this version).
2. Build and load as usual. Both the chosen switch (IRQ 17) and the push-button (IRQ 18) will now call the same `advance_time_seconds(TIME_INCREMENT)` helper with the same debounce delay.
3. If you ever want to disable the button, set `ENABLE_BUTTON_INCREMENT` to `0`. The code still clears its edges but will not change the time.

That is the entire path from reset, through interrupt wiring in `boot.S`, into the C handlers that keep and adjust time.
