# Edge cases

This Markdown file is designed to be your study guide and cheatsheet for the next lab session. It documents your current success with `mul` and provides specific implementation guides for the most likely "surprise" alternatives (`sub`, `bne`, `slt`), tailored to your specific Logisim setup.

---

## 1. The "Mul" Solution (Reference)

This is what you just implemented. Keep this as a reference for the logic pattern.

**Instruction:** `mul rd, rs1, rs2`

**Identification:**
- **Opcode:** `0x33` (51 dec) → Matches R-Type
- **Funct7:** Bit 25 is 1 (This is the differentiator from `add`)

**Hardware Change:** Added Multiplier to ALU

**Control Change:** 
- Added "Bit 25" splitter
- Added logic: `IF (Opcode == 0x33 AND Bit_25 == 1) THEN ALUControl = 100`

---

## 2. The Strategy for ANY New Instruction

Every "surprise" task follows the same 3-step diagnosis:

1. **Check the Green Sheet (Instruction Set):** Compare the new instruction to one you already have.
   - Is the Opcode new? (e.g., `jal` vs `add`)
   - Is the Opcode the same, but Funct3 different? (e.g., `beq` vs `bne`)
   - Is the Opcode and Funct3 the same, but Funct7 different? (e.g., `add` vs `sub`)

2. **Check the ALU:** Does your ALU already have the math? (You said yours has Subtraction already)

3. **Update Control Unit:** Tap into the bits (Opcode, Funct3, or Funct7) to send the correct signal

---

## 3. Likely Scenario A: The `sub` (Subtract) Instruction

**Probability:** Very High. You mentioned your ALU already has subtraction, making this the most logical next step.

### The Analysis

**Instruction:** `sub rd, rs1, rs2`

- **Opcode:** `0x33` (Same as `add` and `mul`)
- **Funct3:** `000` (Same as `add`)
- **Funct7:** `0100000` → Bit 30 is 1 (This is the key difference. `add` has 0 at bit 30)

### Step-by-Step Implementation

**Control Unit - The Splitter:**
- You currently have a splitter on the main instruction checking Bit 25 (for `mul`)
- Add a new splitter (or update the existing one) to check Bit 30

**Control Unit - The Logic:**
- Create an AND Gate
  - **Input A:** The "R-Type" signal (output of the `0x33` comparator)
  - **Input B:** The Bit 30 wire
  - **Output:** This is your `IS_SUB` signal

**Control Unit - The MUX:**
- You need to send the correct ALU Control code for subtraction (usually `110` or `001` depending on your specific ALU design)
- Chain a new MUX before or after your existing `mul` MUX
- **Logic:** If `IS_SUB` is 1, output [Sub Code], else pass previous signal

### Assembly Code Update (Factorial)

Replace the `addi` decrement with `sub`:

```assembly
# Old Way
addi t1, t1, -1

# New Way (using sub)
addi t5, zero, 1    # Put 1 in a temp register
sub  t1, t1, t5     # t1 = t1 - 1
```

---

## 4. Likely Scenario B: The `bne` (Branch Not Equal)

**Probability:** High. This tests your understanding of the "Zero" flag.

### The Analysis

**Instruction:** `bne rs1, rs2, label`

- **Opcode:** `0x63` (Same as `beq`)
- **Funct3:** `001` (Different! `beq` is `000`)

### Step-by-Step Implementation

**Control Unit - The Splitter:**
- You need to inspect Funct3 (Bits 12-14)
- Add a splitter to the main instruction wire to grab Bit 12 (Since `beq` is `000` and `bne` is `001`, bit 12 is enough to tell them apart)

**Control Unit - The Branch Logic:**

Currently, you likely calculate: `Branch_Enable AND Zero_Flag`

You need to modify this. `bne` should jump when the Zero Flag is 0.

**Logic:**
- If Opcode is `0x63` AND Bit 12 is 0 (`beq`): Jump if Zero is 1
- If Opcode is `0x63` AND Bit 12 is 1 (`bne`): Jump if Zero is 0

**The "Easy" Hardware Fix (XOR Gate):**
1. Take the Zero Flag coming from the ALU
2. Take Bit 12 from the instruction (Funct3)
3. Run them through an XOR gate
4. Send the result to the final Branch AND gate

**Why?**
- `beq` (Bit12=0): `Zero XOR 0 = Zero` (Standard behavior)
- `bne` (Bit12=1): `Zero XOR 1 = NOT Zero` (Inverted behavior)

### Assembly Code Update

```assembly
# Old loop logic (Check if equal to exit)
loop:
   beq t1, tp, done
   ...
   beq zero, zero, loop

# New loop logic (Check if NOT equal to continue)
loop:
   ...
   addi t1, t1, 1
   bne t1, tp, loop  # If counter != limit, go back to top
```

---

## 5. Likely Scenario C: The `slt` (Set Less Than)

**Probability:** Medium. Used for proper loops `for(i=0; i<n; i++)`.

### The Analysis

**Instruction:** `slt rd, rs1, rs2`

- **Opcode:** `0x33` (R-Type)
- **Funct3:** `010` (This is the ID)
- **Funct7:** `0000000`

### Implementation Guide

**ALU:** 
- Ensure your ALU has an output for SLT
- Usually, this logic is: `Result = (A - B < 0) ? 1 : 0`
- In Logisim, this is often the "Less Than" output of a Comparator component, or the MSB (Sign bit) of a Subtraction result

**Control Unit:**
1. Split instruction to find Funct3 (Bits 12-14)
2. Detect if Funct3 == `010`
3. If True, send the specific SLT code to the ALU (e.g., `111` or `7`)

---

## 6. Cheatsheet: The "Green Sheet" at a Glance

Since you cannot easily access Google during the surprise, here are the codes you are most likely to need.

| Instruction | Opcode (Hex) | Opcode (Dec) | Funct3 | Funct7 / Other           |
|-------------|--------------|--------------|--------|--------------------------|
| `add`       | `0x33`       | 51           | `000`  | `0000000`                |
| `sub`       | `0x33`       | 51           | `000`  | `0100000` (Bit 30!)      |
| `mul`       | `0x33`       | 51           | `000`  | `0000001` (Bit 25!)      |
| `slt`       | `0x33`       | 51           | `010`  | `0000000`                |
| `xor`       | `0x33`       | 51           | `100`  | `0000000`                |
| `addi`      | `0x13`       | 19           | `000`  | N/A                      |
| `andi`      | `0x13`       | 19           | `111`  | N/A                      |
| `ori`       | `0x13`       | 19           | `110`  | N/A                      |
| `beq`       | `0x63`       | 99           | `000`  | N/A                      |
| `bne`       | `0x63`       | 99           | `001`  | N/A                      |

---

## How to Read Your Logisim Control Unit

- **Top Comparator (`63` hex / `99` dec):** This detects Branches (`beq`, `bne`). Look at bits 12-14 (Funct3) to tell them apart.

- **Middle Comparator (`13` hex / `19` dec):** This detects Immediates (`addi`, `andi`, `ori`). Look at bits 12-14 (Funct3) to tell them apart.

- **Bottom Comparator (`33` hex / `51` dec):** This detects R-Type (`add`, `sub`, `mul`, `slt`). Look at bits 25-31 (Funct7) OR bits 12-14 (Funct3) to tell them apart.
