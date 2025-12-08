## Surprise assignment for lab 4

You need to solve the following surprise assignment and then let a teaching assistant check that the solution is correct and that you understand your solution. It is very important that all students in a lab group completely understand the solution. Hence, you should solve this exercise together and take turn on who is actually using the computer.

**NOTE:** Before you start to edit your processor design, copy the file *processor.circ* into a new file e.g. a file *processor-surprise.circ* and edit this file instead. When you are examined on the other assignments for Lab 4, it is important that you can show the original files that are not updated with the surprise assignment.

### Task

You should add the **mul** RISCV instruction to your single-cycle processor and then update the factorial assembly program to make use of this instruction instead of having a loop to perform the multiplication. Your solution should include the following:

* Update the **ALU** sub-circuit to support 32-bit multiplication. You should use the function input signal F = 100₂ for the multiplication. You may use Logisim's library component for multiplication.
* Update the **ControlUnit** sub-circuit to support the decoding of the RISCV **mul** instruction.
* Create a new assembly program in the RARS simulator that is using the **mul** instruction instead of the “add loop” that you provided in the previous assignment.
* Encode the new program in Logisim (update the instruction memory). Test that the new factorial function works. Also, test that the old factorial program (with the “add loop”) still works with the new circuit.