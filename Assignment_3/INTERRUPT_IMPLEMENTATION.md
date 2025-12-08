# Lab 3 - Flexible Interrupt Implementation

## Overview

This implementation handles interrupts from **TWO independent sources**:
1. **Timer (IRQ 16)** - Triggers every 0.1 seconds to update the clock
2. **Switches (IRQ 17)** - Triggers when a specific switch changes state

The implementation is **flexible** and can be easily configured for different lab assignments by changing two constants at the top of `labmain.c`.

---

## Assignment Configuration

### Assignment #1633 - Switch #3 adds 2 seconds
```c
#define SWITCH_NUMBER 3
#define TIME_INCREMENT 2
```

### Assignment #1632 - Switch #2 adds 3 seconds
```c
#define SWITCH_NUMBER 2
#define TIME_INCREMENT 3
```

Simply change these two values and recompile!

---

## How It Works

### 1. Interrupt Enable (boot.S:138-141)

The `enable_interrupt` function in `boot.S` enables interrupts from **both** sources:

```assembly
li   t1, (1 << 16) | (1 << 17)   # Create mask: bit 16 (timer) + bit 17 (switches)
csrs mie, t1                      # Set both bits in mie register
csrsi mstatus, 3                  # Enable global interrupts (MIE bit)
```

- **Bit 16** in `mie` enables Timer interrupts
- **Bit 17** in `mie` enables Switch interrupts
- **Bit 0** in `mstatus` (MIE) enables global machine interrupts

### 2. Initialization (labinit function)

The `labinit()` function sets up both interrupt sources:

#### Timer Configuration
```c
*timer_periodl = period & 0xFFFF;
*timer_periodh = (period >> 16) & 0xFFFF;
*timer_control = 0b111;  // START + CONTINUOUS + INTERRUPT ENABLE
```

#### Switch GPIO Configuration
```c
*switch_edgecapture = 0xFFFFFFFF;                      // Clear any pending edges
*switch_interruptmask = (1 << SWITCH_BIT_POSITION);    // Enable interrupt for specific switch
```

This configures the GPIO PIO core to:
- Detect edges on the switch input
- Generate an interrupt (IRQ 17) when an edge is detected
- Only monitor the specific switch defined by `SWITCH_NUMBER`

### 3. Interrupt Handling (handle_interrupt function)

The interrupt handler receives a `cause` parameter that indicates which interrupt occurred:

#### Timer Interrupt (cause == 16)
```c
if (cause == 16) {
    *timer_status = 0;              // Acknowledge interrupt
    timeout_counter++;
    if (timeout_counter == 10) {    // Every 1 second
        // Update displays
        tick(&mytime);              // Advance time by 1 second
    }
}
```

#### Switch Interrupt (cause == 17)
```c
else if (cause == 17) {
    int edge_capture = *switch_edgecapture;
    if (edge_capture & (1 << SWITCH_BIT_POSITION)) {
        *switch_edgecapture = (1 << SWITCH_BIT_POSITION);  // Clear edge

        // Advance time by TIME_INCREMENT seconds
        for (int i = 0; i < TIME_INCREMENT; i++) {
            tick(&mytime);
        }

        // Update displays immediately
        // ...

        delay(1000000);  // Debounce delay to prevent multiple triggers
    }
}
```

---

## Hardware Registers

### Timer Registers (Base: 0x04000020)
| Offset | Register | Purpose |
|--------|----------|---------|
| 0 | status | Read: interrupt pending, Write: clear interrupt |
| 1 | control | Timer control (start, continuous, IRQ enable) |
| 2 | periodl | Lower 16 bits of period |
| 3 | periodh | Upper 16 bits of period |

### Switch GPIO PIO Registers (Base: 0x04000010)
| Offset | Register | Purpose |
|--------|----------|---------|
| 0 | data | Read current switch values |
| 2 | interruptmask | Enable/disable interrupts per switch |
| 3 | edgecapture | Edge detection status (write 1 to clear) |

### Button GPIO PIO Registers (Base: 0x040000d0)
| Offset | Register | Purpose |
|--------|----------|---------|
| 0 | data | Read current button values |
| 2 | interruptmask | Enable/disable interrupts per button |
| 3 | edgecapture | Edge detection status (write 1 to clear) |

*(Button interrupts are prepared for future use but currently disabled)*

---

## Important Notes

### Switch Debouncing
The implementation includes a `delay(1000000)` after handling a switch interrupt to prevent multiple triggers from switch bouncing. This gives the mechanical switch time to settle before the next interrupt can be processed.

### Independent Operation
Timer and switch interrupts work **completely independently**:
- Timer interrupts update the clock every second
- Switch interrupts add extra time immediately when triggered
- Neither affects the other's operation

### Future Expansion
The code is ready for button interrupts (IRQ 18):
- Button GPIO registers are defined
- Interrupt handler has a placeholder for `cause == 18`
- Boot.S can easily be extended to enable bit 18 in `mie`

---

## Files Modified

1. **boot.S** (Assignment_3/boot.S:138-141)
   - Changed from enabling only timer (bit 16)
   - Now enables both timer (bit 16) and switches (bit 17)

2. **labmain.c** (Assignment_3/labmain.c)
   - Added GPIO PIO register definitions
   - Updated `labinit()` to configure switch interrupts
   - Updated `handle_interrupt()` to handle multiple interrupt sources
   - Added configuration constants for flexibility

---

## Testing

To test the implementation:

1. **Configure for your assignment** (edit lines 17-18 in labmain.c)
2. **Compile and upload** to the FPGA board
3. **Verify timer**: Clock should advance 1 second every second
4. **Test switch**: Toggle the configured switch (up or down)
   - Clock should immediately jump forward by TIME_INCREMENT seconds
   - Multiple toggles should continue working

---

## Troubleshooting

**Problem:** Switch interrupt triggers multiple times per toggle
- **Solution:** Increase the delay value in line 218

**Problem:** Timer stops working when switch is used
- **Solution:** Check that timer_status is being cleared properly in the timer interrupt handler

**Problem:** Switch doesn't trigger any interrupt
- **Solution:**
  1. Verify SWITCH_NUMBER is set correctly
  2. Check that boot.S enables bit 17 in mie
  3. Ensure switch_interruptmask is set for the correct bit

---

## References

- **DTEK-V Memory Map.md** - Hardware addresses and IRQ numbers
- **Machine_Interrupt_Registers.md** - mie/mip register documentation
- **Machine_Status_Registers.md** - mstatus register documentation
- **GPIO.pdf** - PIO core register map and behavior
- **Exceptions_and_Interrupts.md** - RISC-V interrupt handling

