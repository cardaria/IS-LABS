# Quick Guide: Surprise Assignment Configuration

## Overview
The file `surprise_flexible.c` can handle BOTH surprise assignment variations (#1632 and #1633) by changing just 2 numbers at the top of the file!

## What You Need to Change

Look at the **top of the file** for these two lines:

```c
#define SWITCH_NUMBER 2           // CHANGE THIS: 2 for Switch #2, 3 for Switch #3
#define TIME_INCREMENT 3          // CHANGE THIS: How many seconds to add (2 or 3)
```

## For Assignment #1632 (Switch #2, +3 seconds):
```c
#define SWITCH_NUMBER 2
#define TIME_INCREMENT 3
```

## For Assignment #1633 (Switch #3, +2 seconds):
```c
#define SWITCH_NUMBER 3
#define TIME_INCREMENT 2
```

## That's It!

Just change those two numbers and the code handles everything else:
- Detecting the right switch
- Adding the right amount of time
- Updating the display
- Working alongside the timer interrupts

## How It Works (Simple Explanation)

1. **Timer interrupts** (every 0.1s): Make the clock tick normally, 1 second at a time
2. **Switch interrupts** (when you flip a switch): Jump the time forward by 2 or 3 seconds (depending on your setting)
3. Both work independently - flipping the switch doesn't break the timer!

## Quick Test

1. Compile and run the code
2. Watch the clock tick normally (1 second at a time)
3. Flip Switch #2 or Switch #3 (depending on your configuration)
4. The time should jump forward by 2 or 3 seconds
5. The clock continues ticking normally after the jump

---

**Note**: If the switch triggers multiple times when you flip it, that's normal - the code has a delay to reduce this, but switches can be "bouncy". Just flip it once and wait!
