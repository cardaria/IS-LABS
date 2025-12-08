/* button_labmain.c

   This file written 2024 by Artur Podobas and Pedro Antunes

   For copyright and licensing, see file COPYING

   Minimal implementation for Lab 3 surprise assignment: Timer + Button interrupt
*/

// *** BUTTON CONFIGURATION ***
#define TIME_INCREMENT 3          // CHANGE THIS: How many seconds to add when button is pressed
#define BUTTON_DEBOUNCE_MS 10     // Small delay to filter bounce
#define BUTTON_RELEASE_WAIT_MS 1  // Polling delay while waiting for button release

/* External function declarations - these are defined in other files */
extern void print(const char*);
extern void print_dec(unsigned int);
extern void display_string(char*);
extern void time2string(char*,int);
extern void tick(int*);
extern void delay(int);
extern int nextprime(int);
extern void enable_interrupt(void);

/* Global variables */
int mytime = 0x5957;                    // Current time in BCD format (59:57 = 59 min, 57 sec)
int hours = 0;                          // Current hour (0-23)
int timeout_counter = 0;                // Counts timer interrupts (0-9, each = 0.1s)
int prime = 1234567;                    // Used by main loop to calculate primes
char textstring[] = "text, more text, and even more text!";

/* Hardware I/O register pointers */
volatile int *LED_PTR       = (volatile int *) 0x04000000; // LEDs
volatile int *DISPLAYS_PTR  = (volatile int *) 0x04000050; // 7-segment displays
volatile int *SWITCH_PTR    = (volatile int *) 0x04000010; // Switches (data register, offset 0)
volatile int *BUTTON_PTR    = (volatile int *) 0x040000d0; // Buttons

/* Timer registers */
volatile int *timer_status  = (volatile int *) 0x04000020; // Offset 0
volatile int *timer_control = (volatile int *) 0x04000024; // Offset 1
volatile int *timer_periodl = (volatile int *) 0x04000028; // Offset 2
volatile int *timer_periodh = (volatile int *) 0x0400002C; // Offset 3

/* Button registers (Avalon PIO layout: data, direction, IRQ mask, edge capture) */
volatile int *button_direction     = (volatile int *) (0x040000d0 + 0x04);
volatile int *button_irq_mask      = (volatile int *) (0x040000d0 + 0x08);
volatile int *button_edge_capture  = (volatile int *) (0x040000d0 + 0x0C);

/* Helper function: Display a single digit (0-9) on a 7-segment display */
void set_displays(int display_number, int value) {
  // Each display is 16 bytes apart in memory (4 int-sized steps = 16 bytes)
  volatile int *DISPLAY_PTR = DISPLAYS_PTR + 4 * display_number;
  char pattern;

  // Convert digit to 7-segment pattern
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
    default: pattern = 0xFF; break; // Off for invalid input
  }
  *DISPLAY_PTR = pattern; // Write pattern to the display
}

/* Helper function: Update all 7-segment displays to reflect current time */
void update_displays(void) {
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

/* Helper function: Advance time by N seconds and update displays */
void advance_time_seconds(int seconds) {
  for (int i = 0; i < seconds; i++) {
    tick(&mytime);
    if (mytime == 0x10000) { // After 0x5959 it goes to 0x10000
      mytime = 0;
      hours++;
      if (hours == 24) hours = 0;
    }
  }
  update_displays();
}

/* Initialize interrupts for timer and button */
void labinit(void) {
  // Configure timer for 0.1 second interrupts
  int period = 3000000 - 1;                 // Set the timer period to 3 million cycles (0.1 sec at 30 MHz)
  *timer_periodl = period & 0xFFFF;         // Set the lower 16 bits of the period
  *timer_periodh = (period >> 16) & 0xFFFF; // Set the upper 16 bits of the period
  *timer_control = 0b111;                   // START + CONTINUOUS + INTERRUPT ENABLE (bits 0, 1, 2)

  // Configure button: set as input and enable interrupt
  *button_direction = 0x0;                  // All buttons as input
  *button_edge_capture = 0x1;               // Clear any stale edges
  *button_irq_mask = 0x1;                   // Enable IRQ for button 0 (bit 0)

  // Initialize displays to show the starting time immediately
  update_displays();

  // Enable global interrupts
  enable_interrupt();
}

/* Handle timer and button interrupts */
void handle_interrupt(unsigned cause) {

  // ====== TIMER INTERRUPT (IRQ 16) ======
  if (cause == 16 && (*timer_status & 1)) {
    // Acknowledge timer interrupt by clearing the status bit
    *timer_status = 0;

    timeout_counter++; // Increment the timeout counter (0-9 for deciseconds)

    if (timeout_counter == 10) { // Every 1 second (10 × 0.1s)
      timeout_counter = 0; // Reset the timeout counter
      advance_time_seconds(1);
    }
    return;
  }

  // ====== BUTTON INTERRUPT (IRQ 18) ======
  if (cause == 18) {
    unsigned int edges = *button_edge_capture; // Read latched edges

    if (edges & 0x1) { // Check if button 0 triggered
      // Temporarily disable button interrupts to prevent retriggering
      *button_irq_mask = 0;

      // Clear the edge capture register
      *button_edge_capture = edges;

      // Advance time by configured amount
      advance_time_seconds(TIME_INCREMENT);

      // Debounce: wait for hardware to settle
      delay(BUTTON_DEBOUNCE_MS);

      // Wait for button to be released to prevent multiple increments on hold
      while ((*BUTTON_PTR & 0x1) != 0) {
        delay(BUTTON_RELEASE_WAIT_MS);
      }

      // Clear any residual edges from the release
      *button_edge_capture = *button_edge_capture;

      // Re-enable the button interrupt
      *button_irq_mask = 0x1;
    } else {
      // Clear any other edges
      *button_edge_capture = edges;
    }
    return;
  }
}

/* Main program */
int main(void) {
  labinit(); // Initialize everything ONCE at startup

  while (1) {  // Loop forever
    print("Prime: ");
    prime = nextprime(prime);
    print_dec(prime);
    print("\n");
  }
}
