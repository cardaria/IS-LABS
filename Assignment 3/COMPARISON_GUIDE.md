# Comparison Guide: labmain.c vs surprise_flexible.c

## Overview
This guide shows the **differences** between the original Assignment 3 (`labmain.c`) and the surprise assignment solution (`surprise_flexible.c`).

Both files now have the **same structure and order**, making it easy to compare side-by-side!

---

## File Structure (Both Files)

```
1. Header comment
2. External function declarations
3. Global variables
4. Hardware register pointers
5. Helper functions (set_displays, get_sw, get_btn)
6. labinit() - Initialization function
7. handle_interrupt() - Interrupt handler
8. main() - Main program loop
```

---

## Key Differences Summary

| Feature | labmain.c (Original) | surprise_flexible.c (Surprise) |
|---------|---------------------|--------------------------------|
| **Configuration** | None | Has `SWITCH_NUMBER` and `TIME_INCREMENT` defines |
| **Global variables** | 5 variables | 6 variables (adds `previous_switch_state`) |
| **labinit()** | Only configures timer | Configures timer + saves initial switch state |
| **handle_interrupt()** | Only timer path | Two paths: timer AND switch |
| **Interrupt sources** | Timer only | Timer + Switch interrupts |
| **Timer acknowledgment** | Commented out | Properly acknowledges with `*timer_status = 0` |

---

## Detailed Line-by-Line Differences

### 1. Configuration Section (NEW in surprise_flexible.c)

**labmain.c:**
- No configuration section

**surprise_flexible.c:**
```c
// *** SWITCH CONFIGURATION ***
#define SWITCH_NUMBER 2           // CHANGE THIS: 2 for Switch #2, 3 for Switch #3
#define TIME_INCREMENT 3          // CHANGE THIS: How many seconds to add (2 or 3)
#define SWITCH_BIT_POSITION (SWITCH_NUMBER - 1)
```

**Why the difference?**
- The surprise assignment needs to handle different switch numbers and time increments
- This makes the code flexible without changing multiple places

---

### 2. Global Variables

**labmain.c:**
```c
int mytime = 0x5957;
int hours = 0;
int timeout_counter = 0;
int prime = 1234567;
char textstring[] = "text, more text, and even more text!";
```

**surprise_flexible.c:**
```c
int mytime = 0x5957;
int hours = 0;
int timeout_counter = 0;
int prime = 1234567;
char textstring[] = "text, more text, and even more text!";
int previous_switch_state = 0;  // <--- NEW: Track previous switch state
```

**Why the difference?**
- Need to remember what switches looked like before
- This lets us detect WHICH switch changed, not just that ANY switch changed

---

### 3. labinit() Function

**labmain.c:**
```c
void labinit(void) {
  // Configure timer for 0.1 second interrupts
  int period = 3000000 - 1;
  *timer_periodl = period & 0xFFFF;
  *timer_periodh = (period >> 16) & 0xFFFF;
  *timer_control = 0b111;

  // Enable global interrupts (this enables timer interrupts)
  enable_interrupt();
}
```

**surprise_flexible.c:**
```c
void labinit(void) {
  // Configure timer for 0.1 second interrupts
  int period = 3000000 - 1;
  *timer_periodl = period & 0xFFFF;
  *timer_periodh = (period >> 16) & 0xFFFF;
  *timer_control = 0b111;

  // Store initial switch state  <--- NEW
  previous_switch_state = get_sw();

  // Enable global interrupts (this enables both timer and switch interrupts)
  enable_interrupt();
}
```

**Why the difference?**
- We save the initial switch positions at startup
- Later we can compare current state vs this saved state to detect changes

---

### 4. handle_interrupt() Function - THE BIGGEST DIFFERENCE

#### Part A: Timer Interrupt Handling

**labmain.c:**
```c
void handle_interrupt(unsigned cause) {
  // ====== TIMER INTERRUPT ONLY ======
  // Note: The original code didn't check *timer_status but it's commented out
  // *timer_status = 0; // Acknowledge the timer interrupt
  timeout_counter++;

  if (timeout_counter == 10) {
    timeout_counter = 0;

    // Extract digits and update displays
    int sec_ones = (mytime >> 0) & 0xF;
    // ... etc ...
    set_displays(0, sec_ones);
    // ... etc ...

    // Tick the clock
    tick(&mytime);

    // Handle hour rollover
    if (mytime == 0x10000) {
      mytime = 0;
      hours++;
      if (hours == 24) hours = 0;
    }
  }
}
```

**surprise_flexible.c:**
```c
void handle_interrupt(unsigned cause) {

  // ====== TIMER INTERRUPT (cause is timer) ======
  if (*timer_status & 1) {  <--- NEW: Check if it's actually a timer interrupt
    *timer_status = 0;      <--- NEW: Properly acknowledge the interrupt
    timeout_counter++;

    if (timeout_counter == 10) {
      timeout_counter = 0;

      // Extract digits and update displays
      int sec_ones = (mytime >> 0) & 0xF;
      // ... etc ...
      set_displays(0, sec_ones);
      // ... etc ...

      // Tick the clock
      tick(&mytime);

      // Handle hour rollover
      if (mytime == 0x10000) {
        mytime = 0;
        hours++;
        if (hours == 24) hours = 0;
      }
    }
  }

  // ====== SWITCH INTERRUPT (cause == 17) ======  <--- COMPLETELY NEW SECTION
  if (cause == 17) {
    int current_switch_state = get_sw();
    int switch_changed = current_switch_state ^ previous_switch_state;

    // Check if OUR specific switch changed
    if (switch_changed & (1 << SWITCH_BIT_POSITION)) {
      // Increment mytime by the configured amount
      for (int i = 0; i < TIME_INCREMENT; i++) {
        tick(&mytime);
        if (mytime == 0x10000) {
          mytime = 0;
          hours++;
          if (hours == 24) hours = 0;
        }
      }

      // Update displays immediately
      int sec_ones = (mytime >> 0) & 0xF;
      // ... extract all digits ...
      set_displays(0, sec_ones);
      // ... update all displays ...
    }

    // Update switch state
    previous_switch_state = current_switch_state;

    // Debounce delay
    for (volatile int i = 0; i < 100000; i++);
  }
}
```

**Why the differences?**

1. **Timer interrupt checking**: `if (*timer_status & 1)`
   - Original just assumed every interrupt was from the timer
   - Surprise version needs to distinguish between timer and switch interrupts

2. **Timer acknowledgment**: `*timer_status = 0`
   - Tells the timer "I handled this interrupt"
   - Without this, the interrupt could fire repeatedly

3. **Switch interrupt section**: `if (cause == 17)`
   - COMPLETELY NEW for the surprise assignment
   - Handles switch changes to jump time forward

---

## Visual Comparison: What Gets Added for Surprise Assignment

```
Original (labmain.c):
┌─────────────────────────┐
│  Global Variables       │
│  - mytime               │
│  - hours                │
│  - timeout_counter      │
│  - prime                │
│  - textstring           │
└─────────────────────────┘
           ↓
┌─────────────────────────┐
│  labinit()              │
│  - Configure timer      │
│  - Enable interrupts    │
└─────────────────────────┘
           ↓
┌─────────────────────────┐
│  handle_interrupt()     │
│  - Handle timer only    │
│  - Tick clock           │
└─────────────────────────┘
```

```
Surprise (surprise_flexible.c):
┌─────────────────────────┐
│  Configuration          │  <--- NEW
│  - SWITCH_NUMBER        │
│  - TIME_INCREMENT       │
└─────────────────────────┘
           ↓
┌─────────────────────────┐
│  Global Variables       │
│  - mytime               │
│  - hours                │
│  - timeout_counter      │
│  - prime                │
│  - textstring           │
│  - previous_switch_state│  <--- NEW
└─────────────────────────┘
           ↓
┌─────────────────────────┐
│  labinit()              │
│  - Configure timer      │
│  - Save switch state    │  <--- NEW
│  - Enable interrupts    │
└─────────────────────────┘
           ↓
┌─────────────────────────┐
│  handle_interrupt()     │
│  ┌─ Timer path          │
│  │  - Check timer status│  <--- NEW
│  │  - Acknowledge        │  <--- NEW
│  │  - Tick clock         │
│  └──────────────────────│
│  ┌─ Switch path         │  <--- COMPLETELY NEW
│  │  - Read switches      │
│  │  - Detect changes     │
│  │  - Check our switch   │
│  │  - Tick multiple times│
│  │  - Update displays    │
│  │  - Debounce          │
│  └──────────────────────│
└─────────────────────────┘
```

---

## Quick Reference: What to Study

### If you need to implement the surprise assignment from scratch:

1. **Add these at the top**:
   - Configuration defines (`SWITCH_NUMBER`, `TIME_INCREMENT`)
   - Global variable: `previous_switch_state`

2. **Modify labinit()**:
   - Add: `previous_switch_state = get_sw();`

3. **Modify handle_interrupt()**:
   - Wrap timer code in: `if (*timer_status & 1)`
   - Add: `*timer_status = 0;` at the start of timer section
   - Add entire switch interrupt section: `if (cause == 17)`

4. **Remember the logic**:
   - XOR to detect changes: `switch_changed = current ^ previous`
   - Check specific switch: `switch_changed & (1 << SWITCH_BIT_POSITION)`
   - Loop to tick multiple times: `for (int i = 0; i < TIME_INCREMENT; i++)`

---

## Side-by-Side Function Comparison

### labinit()

| Step | labmain.c | surprise_flexible.c |
|------|-----------|---------------------|
| 1 | Configure timer period | Configure timer period (same) |
| 2 | Enable timer control | Enable timer control (same) |
| 3 | - | **NEW:** Save initial switch state |
| 4 | Enable global interrupts | Enable global interrupts (same) |

### handle_interrupt()

| Section | labmain.c | surprise_flexible.c |
|---------|-----------|---------------------|
| Timer check | Assumes all interrupts are timer | Checks `if (*timer_status & 1)` |
| Timer acknowledgment | Commented out | **NEW:** `*timer_status = 0` |
| Timer logic | Count, display, tick | Same logic |
| Switch handling | None | **NEW:** Entire `if (cause == 17)` section |

---

## Common Mistakes to Avoid

When converting from labmain.c to surprise_flexible.c:

❌ **Forgetting to save initial switch state in labinit()**
- This causes the first switch change to be missed

❌ **Not checking `*timer_status & 1` before handling timer**
- Switch interrupts will incorrectly execute timer code

❌ **Forgetting `*timer_status = 0` acknowledgment**
- Timer may fire repeatedly without counting properly

❌ **Not updating `previous_switch_state` after handling switch**
- Same switch change will be detected repeatedly

❌ **Missing the debounce delay**
- Switch will trigger multiple times from one physical flip

---

## Testing Checklist

After implementing changes:

- [ ] Clock ticks normally (1 second at a time)
- [ ] Flipping the configured switch jumps time by correct amount
- [ ] Flipping other switches does nothing
- [ ] Timer continues working after switch interrupt
- [ ] Switch works while timer is running
- [ ] Both interrupts are independent

---

## Quick Diff Summary

```
ADDITIONS IN surprise_flexible.c:
+ Configuration section (lines 13-18)
+ Global variable: previous_switch_state (line 53)
+ In labinit(): Save initial switch state (line 91)
+ In handle_interrupt(): Check timer status (line 102)
+ In handle_interrupt(): Acknowledge timer (line 103)
+ In handle_interrupt(): Entire switch section (lines 136-176)

MODIFICATIONS IN surprise_flexible.c:
~ Timer code wrapped in if statement (was: runs always, now: checks first)
~ Comments updated to reflect dual interrupt handling
```

---

**Study Tip**: Open both files side-by-side in your editor and compare them section by section. The structure is identical, making it easy to see exactly what changed!
