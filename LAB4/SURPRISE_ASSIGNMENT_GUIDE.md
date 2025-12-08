# Surprise Assignment Implementation Guide

## What I've Done For You:

✅ Created three new assembly files with MUL instruction:
- `assembly files/0fac_mul.asm` - factorial of 0 using MUL
- `assembly files/3fac_mul.asm` - factorial of 3 using MUL
- `assembly files/8fac_mul.asm` - factorial of 8 using MUL

The new programs are **much simpler** - they replaced the entire inner loop (11 lines) with just ONE instruction: `mul t2,t2,t1`

---

## What YOU Need to Do in Logisim:

### Step 1: Backup Your Circuit
1. Open Logisim
2. Open `processor_riscv.circ`
3. Save As → `processor-surprise.circ`
4. Work on this new file

### Step 2: Modify the ALU Sub-Circuit

**What to do:**
1. Open the ALU sub-circuit in Logisim
2. Find where the function code (F input) is decoded
3. Add a new case for F = 4 (binary 100)
4. From the component library, add a **Multiplier** component (Arithmetic → Multiplier)
5. Connect it so when F=4, the multiplier output is selected

**Pseudocode logic:**
```
IF F == 0 THEN output = A + B      (ADD)
IF F == 1 THEN output = A - B      (SUB)
IF F == 2 THEN output = A & B      (AND)
IF F == 3 THEN output = A | B      (OR)
IF F == 4 THEN output = A * B      (MUL) ← ADD THIS!
```

### Step 3: Modify the Control Unit Sub-Circuit

**The MUL instruction encoding (R-type):**
- **opcode**: `0110011` (bits 6-0) = 51 decimal
- **funct3**: `000` (bits 14-12) = 0
- **funct7**: `0000001` (bits 31-25) = 1

**What to do:**
1. Open the Control Unit sub-circuit
2. Find where R-type instructions are decoded (opcode = 0110011)
3. Currently it probably checks funct3 and funct7 to distinguish ADD/SUB/AND/OR
4. Add logic to detect when funct7=`0000001` and funct3=`000`
5. When MUL is detected, set ALUOp (or however you name the signal to ALU) to 4

**Control signals for MUL:**
- ALUSrc = 0 (use register, not immediate)
- MemToReg = 0 (write ALU result to register)
- RegWrite = 1 (write to destination register)
- MemRead = 0
- MemWrite = 0
- Branch = 0
- **ALUOp = 4** (to select multiplication in ALU)

### Step 4: Assemble and Encode the New Program

1. Open one of the new `*_mul.asm` files in RARS simulator
2. Assemble it
3. Look at the machine code (Text Segment in hex)
4. In Logisim, update the Instruction Memory (ROM) with these hex values

**Expected MUL instruction encoding:**
For `mul t2, t2, t1` where:
- rd = t2 = x7
- rs1 = t2 = x7
- rs2 = t1 = x6

Binary: `0000001 00110 00111 000 00111 0110011`
Hex: `0x026383B3`

### Step 5: Test Both Programs

1. Load the NEW program (with MUL) into instruction memory
2. Run it - verify factorial result is correct
3. Load the OLD program (with add loop) into instruction memory
4. Run it - verify it STILL works (proves your circuit is backward compatible)

---

## MUL Instruction Reference

**Format:** `mul rd, rs1, rs2`
**Operation:** `rd = rs1 * rs2` (lower 32 bits of product)

**Encoding (R-type):**
```
31        25 24   20 19   15 14  12 11    7 6      0
[funct7   ] [rs2  ] [rs1  ] [f3  ] [rd   ] [opcode]
[0000001  ] [rs2  ] [rs1  ] [000 ] [rd   ] [0110011]
```

---

## Quick Comparison

**OLD CODE (lines 8-20):**
```assembly
    add  sp,zero,zero       # sp = 0
    add  gp,t1,zero         # gp = i
loop_inner:
    beq  gp,zero,mult_done  # if gp == 0 done
    add  sp,sp,t2           # sp += t2
    addi gp,gp,-1          # gp--
    beq  zero,zero,loop_inner
mult_done:
    add  t2,sp,zero         # t2 = sp
    addi t1,t1,1           # i++
    beq  zero,zero,loop_outer
```

**NEW CODE (lines 7-9):**
```assembly
    mul  t2,t2,t1          # t2 = t2 * i
    addi t1,t1,1           # i++
    beq  zero,zero,loop_outer
```

Much cleaner! 🎉

---

## Troubleshooting

**If the MUL program doesn't work:**
1. Check ALU - does it route to multiplier when F=4?
2. Check Control Unit - does it output ALUOp=4 for MUL instruction?
3. Check instruction encoding - use RARS to verify the hex is correct
4. Use Logisim's "Poke" tool to manually step through and check signals

**Common mistakes:**
- Forgetting funct7 = `0000001` (not `0000000` like ADD)
- Wrong ALUOp value
- Multiplier not connected to ALU output mux
