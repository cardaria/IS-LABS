/* surprise_flexible.c
 *
 * FLEXIBLE SURPRISE ASSIGNMENT SOLUTION
 *
 * This file handles BOTH timer interrupts AND switch interrupts
 * for Lab 3 surprise assignments #1632 and #1633
 *
 * ============================================================
 * CONFIGURATION SECTION - CHANGE THESE VALUES AS NEEDED:
 * ============================================================
 */

// *** SWITCH CONFIGURATION ***
#define SWITCH_NUMBER 2           // CHANGE THIS: 2 for Switch #2, 3 for Switch #3
#define TIME_INCREMENT 3          // CHANGE THIS: How many seconds to add (2 or 3)

// Calculate the bit position for the switch (Switch #1 = bit 0, Switch #2 = bit 1, etc.)
#define SWITCH_BIT_POSITION (SWITCH_NUMBER - 1)

/* ============================================================
 * END OF CONFIGURATION - Don't modify below unless you know what you're doing
 * ============================================================
 */

extern void print(const char*);
extern void print_dec(unsigned int);
extern void display_string(char*);
extern void time2string(char*,int);
extern void tick(int*);
extern void delay(int);
extern int nextprime(int);
extern void enable_interrupt(void);

int mytime = 0x5957;
int hours = 0;
int timeout_counter = 0;
int prime = 1234567;
char textstring[] = "text, more text, and even more text!";

/* Hardware I/O register pointers */
volatile int *LED_PTR       = (volatile int *) 0x04000000;
volatile int *DISPLAYS_PTR  = (volatile int *) 0x04000050;
volatile int *SWITCH_PTR    = (volatile int *) 0x04000010; // Switches located here
volatile int *BUTTON_PTR    = (volatile int *) 0x040000d0;

/* Timer registers */
volatile int *timer_status  = (volatile int *) 0x04000020;
volatile int *timer_control = (volatile int *) 0x04000024;
volatile int *timer_periodl = (volatile int *) 0x04000028;
volatile int *timer_periodh = (volatile int *) 0x0400002C;

/* Track previous switch state to detect changes */
int previous_switch_state = 0;

void set_displays(int display_number, int value) {
  volatile int *DISPLAY_PTR = DISPLAYS_PTR + 4 * display_number;
  char pattern;
  switch(value) {
    case 0: pattern = 0b11000000; break;
    case 1: pattern = 0b11111001; break;
    case 2: pattern = 0b10100100; break;
    case 3: pattern = 0b10110000; break;
    case 4: pattern = 0b10011001; break;
    case 5: pattern = 0b10010010; break;
    case 6: pattern = 0b10000010; break;
    case 7: pattern = 0b11111000; break;
    case 8: pattern = 0b10000000; break;
    case 9: pattern = 0b10010000; break;
    default: pattern = 0xFF; break;
  }
  *DISPLAY_PTR = pattern;
}

int get_sw(void) {
  return *SWITCH_PTR & 0x3FF;
}

int get_btn(void) {
  return *BUTTON_PTR & 0x1;
}

/* Initialize interrupts for BOTH timer and switches */
void labinit(void) {
  // Configure timer for 0.1 second interrupts
  int period = 3000000 - 1; // 0.1s at 30 MHz
  *timer_periodl = period & 0xFFFF;
  *timer_periodh = (period >> 16) & 0xFFFF;
  *timer_control = 0b111; // START + CONTINUOUS + INTERRUPT ENABLE

  // Store initial switch state
  previous_switch_state = get_sw();

  // Enable global interrupts (this enables both timer and switch interrupts)
  enable_interrupt();
}

/* Handle interrupts from BOTH timer (normal ticking) and switches (time jumps) */
void handle_interrupt(unsigned cause) {

  // ====== TIMER INTERRUPT (cause is timer) ======
  // This handles the normal 1-second clock ticking
  if (*timer_status & 1) {
    *timer_status = 0;  // Acknowledge the timer interrupt
    timeout_counter++;

    if (timeout_counter == 10) { // Every 1 second (10 x 0.1s)
      timeout_counter = 0;

      // Extract time digits and display them
      int sec_ones = (mytime >> 0) & 0xF;
      int sec_tens = (mytime >> 4) & 0xF;
      int min_ones = (mytime >> 8) & 0xF;
      int min_tens = (mytime >> 12) & 0xF;
      int hou_ones = hours % 10;
      int hou_tens = hours / 10;

      set_displays(0, sec_ones);
      set_displays(1, sec_tens);
      set_displays(2, min_ones);
      set_displays(3, min_tens);
      set_displays(4, hou_ones);
      set_displays(5, hou_tens);

      // Tick the clock forward by 1 second
      tick(&mytime);

      // Handle hour rollover
      if (mytime == 0x10000) {
        mytime = 0;
        hours++;
        if (hours == 24) hours = 0;
      }
    }
  }

  // ====== SWITCH INTERRUPT (cause == 17) ======
  // This handles the switch toggling to jump time forward
  if (cause == 17) {
    int current_switch_state = get_sw();
    int switch_changed = current_switch_state ^ previous_switch_state;

    // Check if OUR specific switch changed (bit position)
    if (switch_changed & (1 << SWITCH_BIT_POSITION)) {
      // Increment mytime by the configured amount
      // TIME_INCREMENT is in seconds, add directly in BCD
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
      int sec_tens = (mytime >> 4) & 0xF;
      int min_ones = (mytime >> 8) & 0xF;
      int min_tens = (mytime >> 12) & 0xF;
      int hou_ones = hours % 10;
      int hou_tens = hours / 10;

      set_displays(0, sec_ones);
      set_displays(1, sec_tens);
      set_displays(2, min_ones);
      set_displays(3, min_tens);
      set_displays(4, hou_ones);
      set_displays(5, hou_tens);
    }

    // Update switch state
    previous_switch_state = current_switch_state;

    // Debounce delay to prevent multiple triggers
    for (volatile int i = 0; i < 100000; i++);
  }
}

/* Main program */
int main(void) {
  labinit();

  while (1) {
    print("Prime: ");
    prime = nextprime(prime);
    print_dec(prime);
    print("\n");
  }
}
