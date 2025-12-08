# Exceptions and Interrupts (KTH ID1003)

## Introduction to Program Flow
Normal program flow consists of branches and loops [cite: 148-150]. However, flow can be altered by Exceptions and Interrupts[cite: 151].

### Terminology (RISC-V)
We use the RISC-V terminology to differentiate between exceptions and interrupts[cite: 159]. Note that some authors use the term "interrupts" for everything (e.g., used in x86 terminology)[cite: 160].

* **Exceptions:** Caused by error conditions, for instance, division by zero, unknown instruction, etc.[cite: 156].
* **Interrupts:** External interrupts, e.g., keyboard events[cite: 157].
* **Software Interrupts (Traps):** For instance, system calls (from user to supervisory mode)[cite: 158].

---

## Control and Status Registers (CSRs) in RISC-V

RISC-V, aside from the general-purpose registers (0-31), also supports a number of Control and Status (CSR) registers[cite: 174].

### Key CSRs
These registers are used to control local interrupts handled by a local unit called a CLINT, among other tasks[cite: 175, 176]:

* **mstatus:** Machine status control register[cite: 176].
* **mie:** Machine interrupt enable control register[cite: 177].
* **mepc:** Exception/interrupt program counter (holds the address where the exception happened)[cite: 178].
* **mcause:** Indicates the reason why an exception/interrupt happened[cite: 180].
* **mip:** Machine interrupt pending control register[cite: 181].
* **mtvec:** Address to jump to during an exception/interrupt[cite: 182].

### Hardware Performance Counters
These contain machine-specific functionality and support[cite: 183, 184]:
* **cycle:** Cycle counter[cite: 185].
* **instret:** Number of instructions executed[cite: 186].
* **hpmcounterX:** Machine-specific counters[cite: 187].

---

## Accessing CSRs

Accessing the CSRs in RISC-V is performed using specialized instructions[cite: 199]:

* **csrrw:** Reads the old value of the CSR, zero-extends the value, then writes it to integer register `rd`. The initial value in `rs1` is written to the CSR[cite: 200, 201].
* **csrrs:** Reads the value of the CSR, zero-extends the value, and writes it to integer register `rd`. The initial value in integer register `rs1` is treated as a bit mask that specifies bit positions to be **set** in the CSR[cite: 202, 203].
* **csrrc:** Instruction reads the value of the CSR, zero-extends the value, and writes it to integer register `rd`. The initial value in integer register `rs1` is treated as a bit mask that specifies bit positions to be **cleared** in the CSR[cite: 204, 205].

**Note:** All above instructions also come in immediate-forms and using them can be quite complex[cite: 206]. Instead, assemblers often provide easier-to-use pseudo-instructions to access CSRs[cite: 207].

### Code Examples

```assembly
csrsi mie, 17       # Set bit 17 in mie [cite: 209, 210]
csrw mcycle, zero   # Set mcycle to value of r0 (=0) [cite: 211]
csrr a0, mcycle     # Read content of mcycle into a0 [cite: 211]