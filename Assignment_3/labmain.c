/*
 * Flexible lab main for handling both timer and switch/button interrupts.
 *
 * This version supports the surprise assignments in Lab3_Surprise_Assignments.md
 * by letting you choose which switch increments time and by how much. It also
 * prepares the interrupt plumbing for the push button (IRQ 18).
 */

// === CONFIGURATION ===
// Set these values to match the desired surprise assignment.
#define SWITCH_NUMBER 2             // 2 for Switch #2, 3 for Switch #3
#define TIME_INCREMENT 3            // Seconds to add per switch/button interrupt (2 or 3)
#define SWITCH_DEBOUNCE_MS 10       // Small delay to filter bounce without stalling timer
#define ENABLE_BUTTON_INCREMENT 1   // Set to 1 to let the push-button also add TIME_INCREMENT

// Calculate bit position: Switch #1 -> bit 0, Switch #2 -> bit 1, etc.
#define SWITCH_BIT_POSITION (SWITCH_NUMBER - 1)

/* External function declarations - provided by assembly helpers */
extern void print(const char*);
extern void print_dec(unsigned int);
extern void display_string(char*);
extern void time2string(char*, int);
extern void tick(int*);
extern void delay(int);
extern int nextprime(int);
extern void enable_interrupt(void);

/* Global variables */
int mytime = 0x5957;                    // Current time in BCD format (59:57)
int hours = 0;                          // Hours 0-23
int timeout_counter = 0;                // Counts timer interrupts (0-9)
int prime = 1234567;                    // Used by main loop to calculate primes
int previous_switch_state = 0;          // Tracks last switch state for edge detection
char textstring[] = "text, more text, and even more text!";

/* Hardware I/O register pointers */
volatile int *LED_PTR       = (volatile int *) 0x04000000; // LEDs
volatile int *DISPLAYS_PTR  = (volatile int *) 0x04000050; // 7-seg displays
volatile int *SWITCH_PTR    = (volatile int *) 0x04000010; // Switches base
volatile int *BUTTON_PTR    = (volatile int *) 0x040000d0; // Buttons base

/* Timer registers */
volatile int *timer_status  = (volatile int *) 0x04000020; // Offset 0
volatile int *timer_control = (volatile int *) 0x04000024; // Offset 1
volatile int *timer_periodl = (volatile int *) 0x04000028; // Offset 2
volatile int *timer_periodh = (volatile int *) 0x0400002C; // Offset 3

/* Switch register layout (Avalon PIO): data, direction, IRQ mask, edge capture */
volatile int *switch_direction     = (volatile int *) (0x04000010 + 0x04);
volatile int *switch_irq_mask      = (volatile int *) (0x04000010 + 0x08);
volatile int *switch_edge_capture  = (volatile int *) (0x04000010 + 0x0C);

/* Button register layout (Avalon PIO) */
volatile int *button_direction     = (volatile int *) (0x040000d0 + 0x04);
volatile int *button_irq_mask      = (volatile int *) (0x040000d0 + 0x08);
volatile int *button_edge_capture  = (volatile int *) (0x040000d0 + 0x0C);

/* Helper: drive a single 7-seg display */
static void set_displays(int display_number, int value) {
  volatile int *DISPLAY_PTR = DISPLAYS_PTR + 4 * display_number;
  char pattern;

  switch (value) {
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
    default: pattern = 0xFF; break; // Off for invalid input
  }

  *DISPLAY_PTR = pattern;
}

/* Helper: Update all 7-segment displays to reflect current time */
static void update_displays(void) {
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

/* Helper: advance time by a given number of seconds and refresh displays */
static void advance_time_seconds(int seconds) {
  for (int i = 0; i < seconds; i++) {
    tick(&mytime);
    if (mytime == 0x10000) { // rollover from 59:59
      mytime = 0;
      hours = (hours + 1) % 24;
    }
  }
  update_displays();
}

/* Helper function: Read all 10 switches */
static int get_sw(void) {
  return *SWITCH_PTR & 0x3FF; // Return the lowest 10 bits
}

/* Initialize interrupts for timer, switches, and button (armed) */
void labinit(void) {
  // Configure timer for 0.1 second interrupts
  int period = 3000000 - 1;                 // 3 million cycles (0.1 sec at 30 MHz)
  *timer_periodl = period & 0xFFFF;         // Lower 16 bits
  *timer_periodh = (period >> 16) & 0xFFFF; // Upper 16 bits
  *timer_control = 0b111;                   // START + CONTINUOUS + INTERRUPT ENABLE

  // Configure switches as inputs and enable IRQ for the selected switch bit
  *switch_direction = 0x0;                 // All bits as input
  *switch_edge_capture = 0x3FF;            // Clear any stale edges
  *switch_irq_mask = (1 << SWITCH_BIT_POSITION); // Enable interrupt for chosen switch
  previous_switch_state = get_sw();

  // Prepare button block
  *button_direction = 0x0;
  *button_edge_capture = 0x1;              // Clear potential edge
  *button_irq_mask = ENABLE_BUTTON_INCREMENT ? 0x1 : 0x0; // Enable if desired

  // Show the starting time immediately
  update_displays();

  // Enable global + peripheral interrupts
  enable_interrupt();
}

/* Handle timer and GPIO interrupts */
void handle_interrupt(unsigned cause) {
  // TIMER interrupt (IRQ 16)
  if (cause == 16 && (*timer_status & 1)) {
    *timer_status = 0; // acknowledge timer IRQ
    timeout_counter++;
    if (timeout_counter == 10) { // 1 second elapsed
      timeout_counter = 0;
      advance_time_seconds(1);
    }
    return;
  }

  // SWITCH interrupt (IRQ 17)
  if (cause == 17) {
    unsigned int edges = *switch_edge_capture; // latched edges
    if (edges & (1 << SWITCH_BIT_POSITION)) {
      // Temporarily mask to avoid rapid retrigger from switch bounce
      *switch_irq_mask = 0;

      *switch_edge_capture = edges; // clear captured edges
      previous_switch_state = get_sw();
      advance_time_seconds(TIME_INCREMENT);

      // Debounce: allow hardware to settle, then clear any residual edges
      delay(SWITCH_DEBOUNCE_MS);
      *switch_edge_capture = *switch_edge_capture;

      // Re-enable the configured switch interrupt
      *switch_irq_mask = (1 << SWITCH_BIT_POSITION);
    } else {
      *switch_edge_capture = edges; // still clear any other edges
    }
    return;
  }

  // BUTTON interrupt (IRQ 18) - optional time increment
  if (cause == 18) {
    unsigned int edges = *button_edge_capture;

    if (ENABLE_BUTTON_INCREMENT && (edges & 0x1)) {
      *button_irq_mask = 0;             // mask during service
      *button_edge_capture = edges;     // clear latched edge
      advance_time_seconds(TIME_INCREMENT);
      delay(SWITCH_DEBOUNCE_MS);        // reuse same short debounce
      *button_edge_capture = *button_edge_capture; // clear any bounce
      *button_irq_mask = 0x1;           // unmask
    } else {
      *button_edge_capture = edges;     // still acknowledge
    }
    return;
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

