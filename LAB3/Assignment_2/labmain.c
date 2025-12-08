/* main.c

   This file written 2024 by Artur Podobas and Pedro Antunes

   For copyright and licensing, see file COPYING */


/* Below functions are external and found in other files. */
extern void print(const char*);
extern void print_dec(unsigned int);
extern void display_string(char*);
extern void time2string(char*,int);
extern void tick(int*);
extern void delay(int);
extern int nextprime( int );

int mytime = 0x5957;
int hours = 0;
int timeout_counter = 0;
char textstring[] = "text, more text, and even more text!";

/* Below are pointers to the hardware I/O registers. */
volatile int *LED_PTR = (volatile int *) 0x04000000; // Pointer to the address of the LEDs
volatile int *DISPLAYS_PTR = (volatile int *) 0x04000050; // Pointer to the address of the displays
volatile int *SWITCH_PTR = (volatile int *) 0x04000010; // Pointer to the address of the switches
volatile int *BUTTON_PTR = (volatile int *) 0x040000d0; // Pointer to the address of the second button

/* Below are pointers to the timer control registers. */
volatile int *timer_status = (volatile int *) 0x04000020; // Offset 0
volatile int *timer_control = (volatile int *) 0x04000024; // Offset 1
volatile int *timer_periodl = (volatile int *) 0x04000028; // Offset 2 
volatile int *timer_periodh = (volatile int *) 0x0400002C; // Offset 3

/* Below is the function that will be called when an interrupt is triggered. */
void handle_interrupt(unsigned cause) 
{}

/* Add your code here for initializing interrupts. */
void labinit(void){
  int period = 3000000 - 1;                 // Set the timer period to 3 million cycles (0.1 sec at 30 MHz)
  *timer_periodl = period & 0xFFFF;         // Set the lower 16 bits of the period
  *timer_periodh = (period >> 16) & 0xFFFF; // Set the upper 16 bits of the period
  *timer_control = 0b110;                   // Start (START) and set it repeating (CONT) by setting bit index 1 and 2
  return;
}

void set_leds(int led_mask){
  *LED_PTR = led_mask & 0x3FF; // Only use the lowest 10 bits [1023]. "&" is a bitwise AND operation.
}

void set_displays(int display_number, int value){
  volatile int *DISPLAY_PTR = DISPLAYS_PTR + 4 * display_number;// Create a new local pointer that is defined as arg * 4 steps [int = 4 byts] = arg * 16 bytes away from the start
  char pattern; // Determine the pattern for the 7-segment display based on the value
  switch(value){ 
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
  *DISPLAY_PTR = pattern; // Set the pattern to the display
}

int get_sw(void){
  return *SWITCH_PTR & 0x3FF; // Return the lowest 10 bits [1023] // 00000000 00000000 00000011 11111111
}

int get_btn(void){
  return *BUTTON_PTR & 0x1; // Return the lowest 1 bit
  }

/* Your code goes into main as well as any needed functions. */
int main() {
  // Call labinit()
  labinit();

  // Enter a forever loop
  while (1) {
    // Check if the timer interrupt flag is set
    if (*timer_status & 1) {
      *timer_status = 0; // Acknowledge the timer interrupt by writing a 0 to the status register
      timeout_counter++; // Increment the timeout counter

      if (get_btn()) {
        // Check if button is pressed
        int switch_value = get_sw(); // Read the switch value

        int l_switch = (switch_value >> 9) & 1;     // Get the 10th bit
        int r_switch = (switch_value >> 8) & 1;     // Get the 9th bit
        int modify_value = switch_value & 0b111111; // Get the lowest 6 bits
        
        if (l_switch && !r_switch) {       // If left switch = 1 and right switch = 0 - Modify seconds
          if (modify_value < 60) {        // Ensure the value is valid for seconds
            int tens = (modify_value / 10) << 4; // Calculate tens place and shift left by 4 bits
            int ones = modify_value % 10; // Calculate ones place
            mytime = (mytime & 0xFF00) | tens | ones; // Combine all the bits into one new value with only the relevant values for each posistion
          }

        } else if (!l_switch && r_switch) {  // If left switch = 0 and right switch = 1 - Modify minutes
            if (modify_value < 60) {        // Ensure the value is valid for minutes
              int tens = (modify_value / 10) << 12; // Calculate tens place and shift left by 12 bits
              int ones = (modify_value % 10) << 8;  // Calculate ones place and shift left by 8 bits
              mytime = (mytime & 0x00FF) | tens | ones; // Combine all the bits into one new value with only the relevant values for each posistion
          }

        } else if (l_switch && r_switch){  // If both switches are 1 - Modify hours
            if (modify_value < 24) {      // Ensure the value is valid for seconds
              hours = modify_value;       // Set hours directly
          }
        }
      }

      if (timeout_counter == 10) { // Every second (10 * 0.1s)
        timeout_counter = 0; // Reset the timeout counter

        // Extract the digits from mytime and hours
        int sec_ones = (mytime >> 0) & 0xF;
        int sec_tens = (mytime >> 4) & 0xF;
        int min_ones = (mytime >> 8) & 0xF;
        int min_tens = (mytime >> 12) & 0xF;
        int hou_ones = hours % 10;
        int hou_tens = hours / 10;

        // Update the 7-segment displays
        set_displays(0, sec_ones);
        set_displays(1, sec_tens);
        set_displays(2, min_ones);
        set_displays(3, min_tens);
        set_displays(4, hou_ones);
        set_displays(5, hou_tens);
        
        // Tick the clock
        tick( &mytime ); // Ticks the clock once

        // Increment hours
        if (mytime == 0x10000) { // after 0x5959 it goes to 0x10000, then reset mytime and increment hours
            mytime = 0;
            hours++;
            if (hours == 24) hours = 0;
        }
      }
    }
  }
}
