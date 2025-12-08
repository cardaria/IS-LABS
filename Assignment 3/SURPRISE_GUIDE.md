# Complete Guide: Surprise Assignment Implementation

## Table of Contents
1. [Quick Configuration](#quick-configuration)
2. [Understanding Interrupts](#understanding-interrupts)
3. [Program Flow (Chronological Order)](#program-flow-chronological-order)
4. [Code Explanation Section by Section](#code-explanation-section-by-section)
5. [Key Concepts to Remember](#key-concepts-to-remember)
6. [Testing Your Code](#testing-your-code)

---

## Quick Configuration

### What to Change for Different Assignments

Look at **lines 14-15** in `surprise_flexible.c`:

```c
#define SWITCH_NUMBER 2           // Which switch to monitor (2 or 3)
#define TIME_INCREMENT 3          // How many seconds to add (2 or 3)
```

**Assignment #1632** (Switch #2, +3 seconds):
```c
#define SWITCH_NUMBER 2
#define TIME_INCREMENT 3
```

**Assignment #1633** (Switch #3, +2 seconds):
```c
#define SWITCH_NUMBER 3
#define TIME_INCREMENT 2
```

That's it! Everything else is handled automatically.

---

## Understanding Interrupts

### What is an Interrupt?

Think of interrupts like phone calls while you're working:
- **Normal work**: Your main program is calculating prime numbers
- **Phone rings (interrupt)**: The CPU stops what it's doing
- **Answer the phone**: Run the `handle_interrupt()` function
- **Hang up**: Return to calculating primes

### Two Types of Interrupts in This Lab

1. **Timer Interrupt** (happens every 0.1 seconds)
   - Like an alarm clock that beeps every 0.1 seconds
   - We count 10 of these to make 1 second
   - This makes the clock tick normally

2. **Switch Interrupt** (happens when you flip a switch)
   - Cause code = 17 (this identifies it as a switch interrupt)
   - We detect WHICH switch was flipped
   - Jump the time forward by 2 or 3 seconds

---

## Program Flow (Chronological Order)

### Step 1: Program Starts
```
main() is called
  ↓
```

### Step 2: Initialization
```
main() calls labinit()
  ↓
labinit() does:
  1. Configure the timer hardware (period, start it, enable interrupts)
  2. Read the initial switch positions (to detect changes later)
  3. Call enable_interrupt() to turn on global interrupts
  ↓
Return to main()
```

### Step 3: Main Loop Runs Forever
```
while(1) {
    Calculate next prime number
    Print it
}
```

**Important**: This loop NEVER stops! But it gets INTERRUPTED regularly.

### Step 4: Interrupts Happen (While Main Loop is Running)

#### Every 0.1 seconds: Timer Interrupt
```
Timer hardware triggers interrupt
  ↓
CPU pauses main loop
  ↓
handle_interrupt() is called
  ↓
Check: Is this from the timer? YES
  ↓
Increment timeout_counter (count to 10)
  ↓
If timeout_counter == 10 (meaning 1 second passed):
  - Reset counter to 0
  - Update the 7-segment displays
  - Call tick() to advance time by 1 second
  - Check if hour rolled over
  ↓
Return to main loop (resume calculating primes)
```

#### When You Flip a Switch: Switch Interrupt
```
Switch hardware triggers interrupt (cause = 17)
  ↓
CPU pauses main loop
  ↓
handle_interrupt(17) is called with cause = 17
  ↓
Check: Is this from a switch? YES (cause == 17)
  ↓
Read current switch state
  ↓
Compare to previous switch state (detect which switch changed)
  ↓
Did OUR specific switch change? (Check bit position)
  ↓
If YES:
  - Call tick() multiple times (2 or 3 times based on TIME_INCREMENT)
  - Update displays immediately
  - Save new switch state
  - Wait a bit (debounce delay)
  ↓
Return to main loop (resume calculating primes)
```

---

## Code Explanation Section by Section

### Section 1: Configuration (Lines 14-15)

```c
#define SWITCH_NUMBER 2
#define TIME_INCREMENT 3
```

**What it does**: These are like settings you can change easily.

**Why it exists**: So you don't have to search through the code to change which switch or how much time to add.

**How it works**:
- `#define` is a preprocessor command - before compiling, every time the compiler sees `SWITCH_NUMBER`, it replaces it with `2`
- Think of it like "Find and Replace" in a document

---

### Section 2: Global Variables (Lines 32-36)

```c
int mytime = 0x5957;           // Current time in BCD format (59:57 = 59 min, 57 sec)
int hours = 0;                 // Current hour (0-23)
int timeout_counter = 0;       // Counts timer interrupts (0-9)
int prime = 1234567;          // Used by main loop to find primes
int previous_switch_state = 0; // Remember last switch positions
```

**What it does**: Variables that need to be accessed by multiple functions.

**Why global**: Both `main()` and `handle_interrupt()` need to access these.

**Key variable - `mytime`**:
- Stores time in **BCD (Binary Coded Decimal)** format
- `0x5957` means 59 minutes and 57 seconds
- Each digit is stored separately in 4 bits (a hexadecimal digit)
- Example: `0x5957` = `0101 1001 0101 0111` = 59:57

**Key variable - `previous_switch_state`**:
- Remembers what the switches looked like last time we checked
- We compare this to the current state to detect CHANGES
- This is how we know WHICH switch was flipped

---

### Section 3: Hardware Pointers (Lines 38-48)

```c
volatile int *LED_PTR       = (volatile int *) 0x04000000;
volatile int *SWITCH_PTR    = (volatile int *) 0x04000010;
volatile int *timer_status  = (volatile int *) 0x04000020;
// etc.
```

**What it does**: Creates pointers to hardware registers.

**Why `volatile`**: Tells the compiler "this value can change at any time" (by hardware), so don't optimize it away.

**Memory addresses**: Each piece of hardware has a specific address:
- `0x04000010` = where switch data is located
- `0x04000020` = timer status register
- These are fixed by the hardware design

**How to use**:
```c
int switches = *SWITCH_PTR;  // READ the switch values
*LED_PTR = 0xFF;             // WRITE to turn on LEDs
```

---

### Section 4: Helper Functions

#### `set_displays()` - Show a digit on a 7-segment display

```c
void set_displays(int display_number, int value)
```

**What it does**: Converts a number (0-9) into the pattern needed for a 7-segment display.

**Parameters**:
- `display_number`: Which display (0-5, since we have 6 displays for HH:MM:SS)
- `value`: What digit to show (0-9)

**How it works**:
1. Calculate which display's address to use (`DISPLAYS_PTR + 4 * display_number`)
2. Use a switch statement to convert number to 7-segment pattern
   - Example: `0` = `0b11000000` (segments a,b,c,d,e,f on)
   - Example: `1` = `0b11111001` (only segments b,c on)
3. Write the pattern to the display

---

#### `get_sw()` - Read switch positions

```c
int get_sw(void) {
  return *SWITCH_PTR & 0x3FF;
}
```

**What it does**: Reads all 10 switches.

**Why `& 0x3FF`**:
- `0x3FF` = `0000001111111111` in binary (10 bits set)
- This masks to only keep the bottom 10 bits (we only have 10 switches)
- Ignores any junk in the upper bits

**Returns**: An integer where each bit represents a switch:
- Bit 0 = Switch #1
- Bit 1 = Switch #2
- Bit 2 = Switch #3
- etc.

---

### Section 5: `labinit()` - Initialization Function

```c
void labinit(void) {
  // 1. Configure timer
  int period = 3000000 - 1;
  *timer_periodl = period & 0xFFFF;
  *timer_periodh = (period >> 16) & 0xFFFF;
  *timer_control = 0b111;

  // 2. Save initial switch state
  previous_switch_state = get_sw();

  // 3. Enable interrupts globally
  enable_interrupt();
}
```

**What it does**: Sets up everything before the main loop runs.

**Step-by-step**:

**Step 1: Configure the timer**
```c
int period = 3000000 - 1;  // Why 3 million?
```
- The clock runs at 30 MHz (30 million cycles per second)
- We want an interrupt every 0.1 seconds
- 30,000,000 cycles/sec × 0.1 sec = 3,000,000 cycles
- We subtract 1 because the counter counts from 0

```c
*timer_periodl = period & 0xFFFF;        // Lower 16 bits
*timer_periodh = (period >> 16) & 0xFFFF; // Upper 16 bits
```
- The period is 32 bits, but we have two 16-bit registers
- Split it into low and high parts

```c
*timer_control = 0b111;  // Binary: 111 = bits 0, 1, 2 are set
```
- Bit 0: START (start the timer)
- Bit 1: CONT (continuous mode, restart automatically)
- Bit 2: ITO (interrupt enable)

**Step 2: Save initial switch state**
```c
previous_switch_state = get_sw();
```
- Read what the switches look like RIGHT NOW
- Later we'll compare to detect changes

**Step 3: Enable global interrupts**
```c
enable_interrupt();
```
- This is an assembly function that sets the CPU's interrupt enable bit
- Without this, no interrupts will happen at all

---

### Section 6: `handle_interrupt()` - THE MOST IMPORTANT FUNCTION

This function gets called AUTOMATICALLY by the CPU when an interrupt happens.

```c
void handle_interrupt(unsigned cause)
```

**Parameter `cause`**: Tells us WHAT caused the interrupt
- Timer interrupt: Check `*timer_status` register
- Switch interrupt: `cause == 17`

---

#### Part A: Timer Interrupt Handler

```c
if (*timer_status & 1) {
    *timer_status = 0;  // IMPORTANT: Acknowledge the interrupt
    timeout_counter++;

    if (timeout_counter == 10) { // 10 × 0.1s = 1 second
        timeout_counter = 0;

        // Extract digits from time
        int sec_ones = (mytime >> 0) & 0xF;   // Shift 0, mask 4 bits
        int sec_tens = (mytime >> 4) & 0xF;   // Shift 4, mask 4 bits
        int min_ones = (mytime >> 8) & 0xF;   // Shift 8, mask 4 bits
        int min_tens = (mytime >> 12) & 0xF;  // Shift 12, mask 4 bits
        // ... etc

        // Update all 6 displays
        set_displays(0, sec_ones);
        set_displays(1, sec_tens);
        // ... etc

        // Tick time forward by 1 second
        tick(&mytime);

        // Check for hour rollover
        if (mytime == 0x10000) {
            mytime = 0;
            hours++;
            if (hours == 24) hours = 0;
        }
    }
}
```

**Step-by-step explanation**:

**Check if it's a timer interrupt**:
```c
if (*timer_status & 1)
```
- The timer status register has a bit that's set to 1 when it triggered
- We check bit 0 with `& 1`

**CRITICAL - Acknowledge the interrupt**:
```c
*timer_status = 0;
```
- You MUST write 0 to the status register
- This tells the timer "I saw the interrupt, reset it"
- If you don't do this, the interrupt will keep firing forever!

**Count to 10 to make 1 second**:
```c
timeout_counter++;
if (timeout_counter == 10)
```
- Each interrupt = 0.1 seconds
- 10 interrupts = 1 second
- This is how we convert 0.1s interrupts into 1s clock ticks

**Extract BCD digits** (This is tricky but important):
```c
int sec_ones = (mytime >> 0) & 0xF;
```
- `mytime` is in BCD: `0x5957` = 59:57
- `0x5957` in binary: `0101 1001 0101 0111`
- We want the rightmost 4 bits (7): `>> 0` (no shift), `& 0xF` (mask 4 bits)
- Result: `0111` = 7

```c
int sec_tens = (mytime >> 4) & 0xF;
```
- Shift right by 4 bits: `0101 1001 0101` (removed the 0111)
- Mask 4 bits: `0101` = 5
- Result: 5 (tens digit of seconds)

**Update displays**:
```c
set_displays(0, sec_ones);  // Display 0 shows ones of seconds
set_displays(1, sec_tens);  // Display 1 shows tens of seconds
// etc.
```

**Tick the clock forward**:
```c
tick(&mytime);
```
- This is a library function that adds 1 second to the BCD time
- It handles the BCD math (when 09 becomes 10, when 59 becomes 00, etc.)

**Handle hour rollover**:
```c
if (mytime == 0x10000) {  // After 59:59, tick() returns 0x10000
    mytime = 0;           // Reset minutes and seconds
    hours++;              // Increment hours
    if (hours == 24) hours = 0;  // Wrap around at midnight
}
```

---

#### Part B: Switch Interrupt Handler

```c
if (cause == 17) {
    // 1. Read current switch state
    int current_switch_state = get_sw();

    // 2. Detect which switches changed
    int switch_changed = current_switch_state ^ previous_switch_state;

    // 3. Check if OUR specific switch changed
    if (switch_changed & (1 << SWITCH_BIT_POSITION)) {

        // 4. Increment time by TIME_INCREMENT seconds
        for (int i = 0; i < TIME_INCREMENT; i++) {
            tick(&mytime);
            if (mytime == 0x10000) {
                mytime = 0;
                hours++;
                if (hours == 24) hours = 0;
            }
        }

        // 5. Update displays immediately
        int sec_ones = (mytime >> 0) & 0xF;
        // ... extract all digits ...
        set_displays(0, sec_ones);
        // ... update all displays ...
    }

    // 6. Save the new switch state
    previous_switch_state = current_switch_state;

    // 7. Debounce delay
    for (volatile int i = 0; i < 100000; i++);
}
```

**Step-by-step explanation**:

**Step 1: Check if it's a switch interrupt**:
```c
if (cause == 17)
```
- Switches are connected to external interrupt #17
- The `cause` parameter tells us which interrupt fired

**Step 2: Read current switch state**:
```c
int current_switch_state = get_sw();
```
- Get all 10 switch positions right now
- Example: `0b0000000100` means Switch #3 is on, others are off

**Step 3: Detect which switches CHANGED**:
```c
int switch_changed = current_switch_state ^ previous_switch_state;
```
- **XOR operation** (`^`): Returns 1 where bits are DIFFERENT
- Example:
  - Previous: `0b0000000000` (all switches off)
  - Current:  `0b0000000100` (switch #3 is on)
  - XOR:      `0b0000000100` (bit 2 is different)

**Step 4: Check if OUR specific switch changed**:
```c
if (switch_changed & (1 << SWITCH_BIT_POSITION))
```

Let's break this down:
- `SWITCH_NUMBER` = 3 (from config)
- `SWITCH_BIT_POSITION = SWITCH_NUMBER - 1 = 2`
  - Why -1? Switch #1 is bit 0, Switch #2 is bit 1, Switch #3 is bit 2
- `(1 << 2)` = Shift 1 left by 2 positions = `0b0000000100`
- `switch_changed & (1 << 2)` = Check if bit 2 is set in switch_changed

**Example**:
- Configuration: `SWITCH_NUMBER = 3`
- User flips Switch #3
- `switch_changed` = `0b0000000100` (bit 2 is set)
- `(1 << 2)` = `0b0000000100`
- `0b0000000100 & 0b0000000100` = `0b0000000100` (non-zero = TRUE)
- Condition passes! We detected Switch #3 changed

**Step 5: Increment time**:
```c
for (int i = 0; i < TIME_INCREMENT; i++) {
    tick(&mytime);
    // ... handle hour rollover ...
}
```
- If `TIME_INCREMENT = 3`, this calls `tick()` 3 times
- Each `tick()` adds 1 second
- Total: time jumps forward by 3 seconds

**Step 6: Update displays immediately**:
```c
int sec_ones = (mytime >> 0) & 0xF;
// ... extract all digits ...
set_displays(0, sec_ones);
// ... update all displays ...
```
- Same as in the timer handler
- We do this immediately so you see the time jump on the displays

**Step 7: Save new switch state**:
```c
previous_switch_state = current_switch_state;
```
- Update our memory of what the switches look like
- Next time, we'll compare against THIS state

**Step 8: Debounce delay**:
```c
for (volatile int i = 0; i < 100000; i++);
```
- **Why?** Physical switches "bounce" - they rapidly open/close when you flip them
- This creates multiple interrupt triggers
- The delay gives the switch time to settle
- `volatile` tells compiler: don't optimize this loop away

---

### Section 7: `main()` Function

```c
int main(void) {
    labinit();  // Initialize everything ONCE at startup

    while (1) {  // Loop forever
        print("Prime: ");
        prime = nextprime(prime);
        print_dec(prime);
        print("\n");
    }
}
```

**What it does**:
1. Call `labinit()` to set everything up
2. Loop forever calculating and printing prime numbers

**Why this doesn't block interrupts**:
- The CPU can be interrupted at ANY time during this loop
- When an interrupt happens:
  1. CPU saves where it was in the loop
  2. Runs `handle_interrupt()`
  3. Returns to exactly where it left off
- The main loop doesn't even know it was interrupted!

---

## Key Concepts to Remember

### 1. BCD (Binary Coded Decimal) Time Format

**Normal binary**: `59 = 0b111011` (binary value)
**BCD**: `59 = 0x59 = 0101 1001` (each decimal digit stored separately)

**Why BCD?**
- Easy to extract individual digits for 7-segment displays
- Each hex digit IS a decimal digit

**Example - `mytime = 0x5957`**:
```
Hex:    5    9    5    7
Binary: 0101 1001 0101 0111
Means:  59 minutes, 57 seconds
```

**Extracting digits**:
```c
// Get rightmost digit (7):
int ones = (0x5957 >> 0) & 0xF;  // 0xF = 0b1111 (4 bits)

// Get second digit (5):
int tens = (0x5957 >> 4) & 0xF;

// Get third digit (9):
int hundreds = (0x5957 >> 8) & 0xF;

// Get fourth digit (5):
int thousands = (0x5957 >> 12) & 0xF;
```

---

### 2. Bit Manipulation Tricks

**Checking a single bit**:
```c
if (value & (1 << n))  // Is bit n set?
```
Example: Check if bit 2 is set in `0b0000000100`
```c
0b0000000100 & (1 << 2)
= 0b0000000100 & 0b0000000100
= 0b0000000100 (non-zero = TRUE)
```

**XOR for detecting changes**:
```c
int changed = new_value ^ old_value;
```
- Returns 1 where bits are different
- Returns 0 where bits are the same

**Masking bits**:
```c
int lower_10_bits = value & 0x3FF;  // 0x3FF = 0b1111111111
int lower_4_bits = value & 0xF;     // 0xF = 0b1111
```

---

### 3. Interrupt Flow

```
Main program running
    ↓
Interrupt happens (timer or switch)
    ↓
CPU AUTOMATICALLY:
  - Saves current program state
  - Calls handle_interrupt()
    ↓
handle_interrupt() runs:
  - Identifies interrupt source
  - Does necessary work
  - MUST acknowledge interrupt
  - Returns
    ↓
CPU AUTOMATICALLY:
  - Restores program state
  - Continues where it left off
```

**Key points**:
- Interrupts can happen at ANY time
- Multiple interrupt sources can exist
- Must acknowledge interrupts (or they repeat forever)
- Keep interrupt handlers FAST (they're blocking other code)

---

### 4. Why We Need `previous_switch_state`

**Problem**: Switch interrupts fire when ANY switch changes, not just ours.

**Solution**:
1. Remember what switches looked like before (`previous_switch_state`)
2. Read what they look like now (`current_switch_state`)
3. XOR to find which changed (`switch_changed = current ^ previous`)
4. Check if the bit for OUR switch is set

**Example**:
- We care about Switch #3 (bit 2)
- User flips Switch #5 (bit 4)
- Without checking: We'd incorrectly respond to Switch #5
- With checking: We ignore Switch #5, only respond to Switch #3

---

### 5. Switch Bit Positions

```
Switch #1 → Bit 0 → (1 << 0) = 0b0000000001
Switch #2 → Bit 1 → (1 << 1) = 0b0000000010
Switch #3 → Bit 2 → (1 << 2) = 0b0000000100
Switch #4 → Bit 3 → (1 << 3) = 0b0000001000
... and so on
```

**General formula**:
```c
Bit position = Switch number - 1
```

That's why we have:
```c
#define SWITCH_BIT_POSITION (SWITCH_NUMBER - 1)
```

---

## Testing Your Code

### What You Should See:

1. **Normal operation**:
   - Displays show time ticking: 00:00, 00:01, 00:02, etc.
   - Primes printing in the terminal

2. **Flip Switch #2 (if configured for it)**:
   - Time immediately jumps forward by 3 seconds (or 2, depending on config)
   - Example: If showing 00:05, it jumps to 00:08
   - Clock continues ticking normally after the jump

3. **Both work together**:
   - Flip the switch while the clock is ticking
   - Time jumps, then continues ticking
   - Prime calculation is not affected

### Common Issues:

**Time jumps too much when you flip the switch**:
- Switch is bouncing (creating multiple interrupts)
- Try increasing the debounce delay

**Nothing happens when you flip the switch**:
- Check you're flipping the RIGHT switch (#2 or #3)
- Check `SWITCH_NUMBER` configuration
- Make sure interrupts are enabled in `labinit()`

**Clock doesn't tick at all**:
- Check timer initialization in `labinit()`
- Check `enable_interrupt()` is being called
- Check you're acknowledging timer interrupt (`*timer_status = 0`)

---

## Memory Aid - The Big Picture

### Configuration Section
```
Set SWITCH_NUMBER and TIME_INCREMENT → These control behavior
```

### Hardware Pointers
```
Point to memory addresses where hardware lives
```

### labinit() - Setup
```
1. Configure timer (period, start, enable)
2. Save initial switches
3. Enable global interrupts
```

### handle_interrupt() - Two paths
```
Timer path:
  Count to 10 → Update displays → Tick time → Check hour rollover

Switch path:
  Read switches → Find changes → Check if ours → Tick multiple times → Update displays → Debounce
```

### main() - Background work
```
Initialize once → Loop forever doing busy work (primes)
Gets interrupted regularly by timer and switches
```

---

## Summary Checklist

Before the surprise assignment, make sure you understand:

- [ ] What BCD format is and how to extract digits
- [ ] How to configure the timer (period registers, control register)
- [ ] Why we acknowledge interrupts (`*timer_status = 0`)
- [ ] How XOR detects changes in switches
- [ ] Why we check `(1 << SWITCH_BIT_POSITION)` for our specific switch
- [ ] Why we need `previous_switch_state`
- [ ] What `volatile` means (hardware can change it)
- [ ] How to change `SWITCH_NUMBER` and `TIME_INCREMENT` for different assignments

**Good luck!** 🍀
