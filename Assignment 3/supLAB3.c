/* main.c

   This file written 2024 by Artur Podobas and Pedro Antunes
   Modified for surprise assignment #1632
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
volatile int *SWITCH_PTR    = (volatile int *) 0x04000010; // Switches (Switch #2 at bit position 1)
volatile int *BUTTON_PTR    = (volatile int *) 0x040000d0;

/* Timer registers */
volatile int *timer_status  = (volatile int *) 0x04000020;
volatile int *timer_control = (volatile int *) 0x04000024;
volatile int *timer_periodl = (volatile int *) 0x04000028;
volatile int *timer_periodh = (volatile int *) 0x0400002C;

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

/* Initialize interrupts */
void labinit(void) {
  int period = 3000000 - 1; // 0.1s at 30 MHz
  *timer_periodl = period & 0xFFFF;
  *timer_periodh = (period >> 16) & 0xFFFF;
  *timer_control = 0b111; // START + CONTINUOUS + INTERRUPT ENABLE
  enable_interrupt();     // Enable global interrupts
}

/* Interrupt handler */
void handle_interrupt(unsigned cause) {
  // TIMER interrupt
  if (*timer_status & 1) {
    *timer_status = 0;  // Acknowledge timer
    timeout_counter++;

    if (timeout_counter == 10) {
      timeout_counter = 0;

      // Display update and ticking
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

      tick(&mytime);

      if (mytime == 0x10000) {
        mytime = 0;
        hours++;
        if (hours == 24) hours = 0;
      }
    }
  }

  // SWITCH #2 interrupt (cause == 17)
  if (cause == 17) {
    mytime += 0x3;     // Simulate 3 seconds passed
    mytime &= 0xFFFF;

    // Simple debounce delay
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
