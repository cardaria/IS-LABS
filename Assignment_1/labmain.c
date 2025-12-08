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
char textstring[] = "text, more text, and even more text!";
volatile int *LED_PTR = (volatile int *) 0x04000000; // Pointer to the address of the LEDs
volatile int *DISPLAYS_PTR = (volatile int *) 0x04000050; // Pointer to the address of the displays
volatile int *SWITCH_PTR = (volatile int *) 0x04000010; // Pointer to the address of the switches
volatile int *BUTTON_PTR = (volatile int *) 0x040000d0; // Pointer to the address of the second button 

/* Below is the function that will be called when an interrupt is triggered. */
void handle_interrupt(unsigned cause) 
{}

/* Add your code here for initializing interrupts. */
void labinit(void)
{}

void set_leds(int led_mask){
  *LED_PTR = led_mask & 0x3FF; // Only use the lowest 10 bits [1023]. "&" is a bitwise AND operation.
}

void set_displays(int display_number, int value){
  // Pointer arithmetic: adding 4 to an int* moves 16 bytes (4 ints × 4 bytes each)
  // Display addresses: 0x04000050, 0x04000060, 0x04000070, ... (0x10 byte spacing, where 0x10 = 16 in decimal)
  volatile int *DISPLAY_PTR = DISPLAYS_PTR + 4 * display_number; 
  char pattern; // Determine the pattern for the 7-segment display based on the value - 7 segment is 8 bits (1 byte) as is a char
  switch(value){ // Each case corresponds to a digit 0-9
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

  /* ASSIGNMENT 1.D */

  int led_counter = 0; // Counter to keep track of LED state

  set_leds(led_counter); // Initialize LEDs to 0

  while(led_counter < 15){ 
    delay(1200); // Delay for 1 second
    led_counter++; // Increment the counter
    set_leds(led_counter); // Update the LEDs
  }

  delay(1200); // Delay for 1 second
  set_leds(0); // Turn off all LEDs

  /* ASSIGNMENT 1.D END */
 
  // infinite loop
  while (1) {
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
    
    // Delay and tick
    delay( 1200 );   // Delays aprox 1 sec
    tick( &mytime ); // Ticks the clock once

    // Increment hours
    if (mytime == 0x10000) { // after 0x5959 it goes to 0x10000, then reset mytime and increment hours
        mytime = 0;
        hours++;
        if (hours == 24) hours = 0; // Reset hours after 23
    }

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
          mytime = (mytime & 0xFF00) | tens | ones; // | = Bitwise OR: Keep the minutes, modify only the seconds 0xFF00 = 11111111 11111111 00000000 00000000

      } else if (!l_switch && r_switch) {  // If left switch = 0 and right switch = 1 - Modify minutes
          if (modify_value < 60) {        // Ensure the value is valid for minutes
            int tens = (modify_value / 10) << 12; // Calculate tens place and shift left by 12 bits
            int ones = (modify_value % 10) << 8;  // Calculate ones place and shift left by 8 bits
            mytime = (mytime & 0x00FF) | tens | ones; // | = Bitwise OR: Keep the seconds, modify only the minutes 0x00FF = 00000000 00000000 11111111 11111111
        }

      } else if (l_switch && r_switch){  // If both switches are 1 - Modify hours
          if (modify_value < 24) {      // Ensure the value is valid for hours
            hours = modify_value;       // Set hours directly
        }
      }
    }
  }
}
