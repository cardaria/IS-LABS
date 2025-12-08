# [cite_start]3.1.9 Machine Interrupt Registers (mip and mie) [cite: 26]

[cite_start]The mip register is an MXLEN-bit read/write register containing information on pending interrupts, while mie is the corresponding MXLEN-bit read/write register containing interrupt enable bits[cite: 27]. [cite_start]Interrupt cause number *i* (as reported in CSR mcause, Section 3.1.15) corresponds with bit *i* in both mip and mie[cite: 30]. [cite_start]Bits 15:0 are allocated to standard interrupt causes only, while bits 16 and above are designated for platform or custom use[cite: 31].

### Register Layouts

[cite_start]**Figure 3.12: Machine Interrupt-Pending Register (mip)** [cite: 35]
* [cite_start]MXLEN-1 to 0: Interrupts (WARL) [cite: 32, 33, 34]

[cite_start]**Figure 3.13: Machine Interrupt-Enable Register (mie)** [cite: 39]
* [cite_start]MXLEN-1 to 0: Interrupts (WARL) [cite: 36, 37, 38]

### Interrupt Trap Conditions

An interrupt *i* will trap to M-mode (causing the privilege mode to change to M-mode) if all of the following are true:
1.  [cite_start]Either the current privilege mode is M and the MIE bit in the mstatus register is set, or the current privilege mode has less privilege than M-mode[cite: 40].
2.  [cite_start]Bit *i* is set in both mip and mie[cite: 41].
3.  [cite_start]If register mideleg exists, bit *i* is not set in mideleg[cite: 42].

[cite_start]These conditions for an interrupt trap to occur must be evaluated in a bounded amount of time from when an interrupt becomes, or ceases to be, pending in mip, and must also be evaluated immediately following the execution of an xRET instruction or an explicit write to a CSR on which these interrupt trap conditions expressly depend (including mip, mie, mstatus, and mideleg)[cite: 43].

[cite_start]Interrupts to M-mode take priority over any interrupts to lower privilege modes[cite: 44].

### Bit Behavior

[cite_start]Each individual bit in register mip may be writable or may be read-only[cite: 45]. [cite_start]When bit *i* in mip is writable, a pending interrupt *i* can be cleared by writing 0 to this bit[cite: 46]. [cite_start]If interrupt *i* can become pending but bit *i* in mip is read-only, the implementation must provide some other mechanism for clearing the pending interrupt[cite: 47].

[cite_start]A bit in mie must be writable if the corresponding interrupt can ever become pending[cite: 48]. [cite_start]Bits of mie that are not writable must be read-only zero[cite: 49].

[cite_start]The standard portions (bits 15:0) of registers mip and mie are formatted as shown in Figures 3.14 and 3.15 respectively[cite: 50].

[cite_start]**Figure 3.14: Standard portion (bits 15:0) of mip** [cite: 71]
* [cite_start]Includes bits for MEIP, SEIP, MTIP, STIP, MSIP, SSIP[cite: 63].

[cite_start]**Figure 3.15: Standard portion (bits 15:0) of mie** [cite: 90]
* [cite_start]Includes bits for MEIE, SEIE, MTIE, STIE, MSIE, SSIE[cite: 84].

### Interrupt Handling Details

[cite_start]The machine-level interrupt registers handle a few root interrupt sources which are assigned a fixed service priority for simplicity, while separate external interrupt controllers can implement a more complex prioritization scheme over a much larger set of interrupts that are then muxed into the machine-level interrupt sources[cite: 92]. [cite_start]The non-maskable interrupt is not made visible via the mip register as its presence is implicitly known when executing the NMI trap handler[cite: 93].

#### Machine-Level Interrupts
* [cite_start]**External Interrupts (MEIP/MEIE):** Bits mip.MEIP and mie.MEIE are the interrupt-pending and interrupt-enable bits for machine-level external interrupts[cite: 95]. [cite_start]MEIP is read-only in mip, and is set and cleared by a platform-specific interrupt controller[cite: 96].
* [cite_start]**Timer Interrupts (MTIP/MTIE):** Bits mip.MTIP and mie.MTIE are the interrupt-pending and interrupt-enable bits for machine timer interrupts[cite: 97]. [cite_start]MTIP is read-only in mip, and is cleared by writing to the memory-mapped machine-mode timer compare register[cite: 98].
* [cite_start]**Software Interrupts (MSIP/MSIE):** Bits mip.MSIP and mie.MSIE are the interrupt-pending and interrupt-enable bits for machine-level software interrupts[cite: 99]. [cite_start]MSIP is read-only in mip, and is written by accesses to memory-mapped control registers, which are used by remote harts to provide machine-level interprocessor interrupts[cite: 100]. [cite_start]A hart can write its own MSIP bit using the same memory-mapped control register[cite: 101]. [cite_start]If a system has only one hart, or if a platform standard supports the delivery of machine-level interprocessor interrupts through external interrupts (MEI) instead, then mip.MSIP and mie.MSIE may both be read-only zeros[cite: 102, 103].

#### Supervisor-Level Interrupts
[cite_start]If supervisor mode is not implemented, bits SEIP, STIP, and SSIP of mip and SEIE, STIE, and SSIE of mie are read-only zeros[cite: 104].

If supervisor mode is implemented:
* [cite_start]**External Interrupts (SEIP/SEIE):** Bits mip.SEIP and mie.SEIE are the interrupt-pending and interrupt-enable bits for supervisor-level external interrupts[cite: 105]. [cite_start]SEIP is writable in mip, and may be written by M-mode software to indicate to S-mode that an external interrupt is pending[cite: 106]. [cite_start]Additionally, the platform-level interrupt controller may generate supervisor-level external interrupts[cite: 107].
    * [cite_start]Supervisor-level external interrupts are made pending based on the logical-OR of the software-writable SEIP bit and the signal from the external interrupt controller[cite: 108].
    * [cite_start]When mip is read with a CSR instruction, the value of the SEIP bit returned in the destination register is the logical-OR of the software-writable bit and the interrupt signal from the interrupt controller, but the signal from the interrupt controller is not used to calculate the value written to SEIP[cite: 109].
    * [cite_start]Only the software-writable SEIP bit participates in the read-modify-write sequence of a CSRRS or CSRRC instruction[cite: 110].
    * [cite_start]The SEIP field behavior is designed to allow a higher privilege layer to mimic external interrupts cleanly, without losing any real external interrupts[cite: 115]. [cite_start]The behavior of the CSR instructions is slightly modified from regular CSR accesses as a result[cite: 116].
* [cite_start]**Timer Interrupts (STIP/STIE):** Bits mip.STIP and mie.STIE are the interrupt-pending and interrupt-enable bits for supervisor-level timer interrupts[cite: 119]. [cite_start]STIP is writable in mip, and may be written by M-mode software to deliver timer interrupts to S-mode[cite: 120].
* [cite_start]**Software Interrupts (SSIP/SSIE):** Bits mip.SSIP and mie.SSIE are the interrupt-pending and interrupt-enable bits for supervisor-level software interrupts[cite: 121]. [cite_start]SSIP is writable in mip and may also be set to 1 by a platform-specific interrupt controller[cite: 122].

### Priority Ordering

[cite_start]Multiple simultaneous interrupts destined for M-mode are handled in the following decreasing priority order: MEI, MSI, MTI, SEI, SSI, STI[cite: 123].

[cite_start]The machine-level interrupt fixed-priority ordering rules were developed with the following rationale[cite: 124]:
* [cite_start]Interrupts for higher privilege modes must be serviced before interrupts for lower privilege modes to support preemption[cite: 125].
* [cite_start]The platform-specific machine-level interrupt sources in bits 16 and above have platform-specific priority, but are typically chosen to have the highest service priority to support very fast local vectored interrupts[cite: 126].
* [cite_start]External interrupts are handled before internal (timer/software) interrupts as external interrupts are usually generated by devices that might require low interrupt service times[cite: 127].
* [cite_start]Software interrupts are handled before internal timer interrupts, because internal timer interrupts are usually intended for time slicing, where time precision is less important, whereas software interrupts are used for inter-processor messaging[cite: 128]. [cite_start]Software interrupts can be avoided when high-precision timing is required, or high-precision timer interrupts can be routed via a different interrupt path[cite: 129].
* [cite_start]Software interrupts are located in the lowest four bits of mip as these are often written by software, and this position allows the use of a single CSR instruction with a five-bit immediate[cite: 130].

### Supervisor Views (sip and sie)

[cite_start]Restricted views of the mip and mie registers appear as the sip and sie registers for supervisor level[cite: 131]. [cite_start]If an interrupt is delegated to S-mode by setting a bit in the mideleg register, it becomes visible in the sip register and is maskable using the sie register[cite: 132]. [cite_start]Otherwise, the corresponding bits in sip and sie are read-only zero[cite: 133].