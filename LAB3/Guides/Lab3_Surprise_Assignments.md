# Surprise assignment for lab 3 #1633

## Task
Change the functions `labinit` and `handle_interrupt` to handle interrupts from two sources: the `Timer` (as before) plus `Switch #3` (new). Let each `Switch #3` interrupt increase the `mytime` variable by 2 (as if 2 seconds passed). 

*Note: you might have to add a small delay in the `handle_interrupt` routine since the interrupts can trigger multiple times when you move the switch position.*

Switches are connected to external interrupt `#17` (`cause=17`) and are located at `0x04000010`.
The interrupts must work independently. If one affects the other, then you’ve done something wrong.

---

# Surprise assignment for lab 3 #1632

## Task
Change the functions `labinit` and `handle_interrupt` to handle interrupts from two sources: the `Timer` (as before) plus the `Switch #2` (new). Let each `Switch #2` interrupt increase the `mytime` variable by 3 (as if 3 seconds passed). 

*Note: you might have to add a small delay in the `handle_interrupt` routine since the interrupts can trigger multiple times when you move the switch position.*

Switches are connected to external interrupt `#17` (`cause=17`) and are located at `0x04000010`.
The interrupts must work independently. If one affects the other, then you’ve done something wrong.