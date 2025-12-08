# Lab 3 Interrupt Walkthrough

This note documents what was added in the latest iteration of `labmain.c` and the relevant interrupt setup in `boot.S`. It also traces the full flow from startup through interrupt handling.

## What changed and why
- **Flexible surprise-assignment config:** The top of `labmain.c` exposes `SWITCH_NUMBER` and `TIME_INCREMENT` so you can choose which switch (IRQ 17) advances the clock and by how many seconds for the Lab 3 variants.
- **Debounced switch handling:** The switch ISR masks the line, clears latched edges, advances time, waits a short `SWITCH_DEBOUNCE_MS`, clears residual edges, and then re-enables the mask so timer ticks keep flowing while bounce is filtered.
- **Display helper restored:** The compact `set_displays` helper updates each 7-seg digit, keeping refresh logic short while mapping the correct segment patterns.
- **Button plumbing ready:** The button block (IRQ 18) is initialized and acknowledged in the ISR even though no behavior is attached yet, preventing spurious retriggers.

## Execution flow (high level)
1. **Reset path in `boot.S`:** `_start` disables interrupts, sets up stack/global pointers, prints a welcome string, then jumps to `main`.
2. **Initialization (`labinit`):**
   - Programs the Avalon timer for 0.1 s ticks and enables its IRQ bit.
   - Sets switches as inputs, clears edge flags, and unmasks only the configured switch bit for IRQ 17.
   - Initializes the button PIO as input, clears its edge flag, and leaves its mask disabled until needed.
   - Renders the starting time and finally calls `enable_interrupt()` to turn on global + external IRQs.
3. **Main loop:** Continuously prints successive primes. Timekeeping is entirely interrupt-driven.

## Interrupt handling details
- **Vector setup (`enable_interrupt`):** Writes `_isr_handler` to `mtvec`, sets the `mie` bits for IRQs 16 (timer), 17 (switches), and 18 (button), then raises the global `mstatus.MIE` flag so interrupts can fire.
- **Entry/exit (`_isr_handler`):** On any trap, registers (except `sp`) are saved, `mcause` is inspected, and external interrupts branch to `handle_interrupt` with the IRQ number (bit 31 cleared). Registers are restored and `mret` returns to the interrupted code.
- **Timer IRQ (16):** The ISR acknowledges the timer, counts ten 0.1 s ticks, advances the time by one second, and refreshes displays—no blocking delays occur here.
- **Switch IRQ (17):** The ISR masks switch interrupts, clears the captured edge, updates `previous_switch_state`, advances by `TIME_INCREMENT`, delays briefly for debounce, clears any bounce edge, and re-enables the chosen switch bit before exiting.
- **Button IRQ (18):** Simply clears the edge-capture register so a future behavior can be added without repeated firing.

## Data path through `labmain.c`
- **Time representation:** `mytime` holds BCD seconds/minutes, while `hours` tracks 24-hour wraparound separately. `advance_time_seconds` repeatedly calls `tick`, handles rollover from 59:59 to 00:00, increments `hours`, and then refreshes displays.
- **Display refresh:** `update_displays` extracts BCD digits plus hour digits and calls `set_displays` for each of the six positions in order (seconds to hours).
- **State tracking:** `previous_switch_state` keeps the last read switch state for edge awareness, while `timeout_counter` tracks timer ticks between second updates.

Keep the `SWITCH_NUMBER`, `TIME_INCREMENT`, and `SWITCH_DEBOUNCE_MS` constants at the top of `labmain.c` aligned with your current assignment variant to avoid surprises.
