# timetemplate.S
# Written 2015 by F Lundevall
# Ported 2024/06 by W Szczerek (from MIPS to RISC-V)
# Copyright abandonded - this file is in the public domain.

#############################################################
# Choose the macro syntax for RARS or DTEK-V board.         #
# The syntax for RARS is probably due to its MIPS heritage. #
#############################################################
###################
# Macros for RARS #
###################
#.macro	PUSH (%reg)
#	addi	sp,sp,-4
#	sw	%reg,0(sp) 
#.end_macro
#
#.macro	POP (%reg)
#	lw	%reg,0(sp)
#	addi	sp,sp,4
#.end_macro
###################
# Macros for DTEK #
###################
.macro	PUSH reg
	addi sp,sp,-4
	sw \reg,0(sp) 
.endm

.macro	POP reg
	lw	\reg,0(sp)
	addi	sp,sp,4
.endm
#############################################################

	.data
	.align 2
mytime:	.word 	0x5957
timstr: .space 6

	.text
	.globl timetemplate, tick, time2string, delay, tick, display_string

# Function for displaying a string with a newline at the end	
display_string:	
	li a7,4 # 4 = print string
	ecall
	li a0, 10 # 10 i ascii  r newline, eller line feed LF
	li a7,11
	ecall
	jr ra
	
timetemplate:
	la	a0, timstr
	jal     display_string
	
	# wait a little
	li	a0, 1000		# ms
	jal	delay
	
	# call tick
	la 	a0, mytime
	jal	tick
	
	# call your function time2string
	la	a0, timstr
	la	t0, mytime
	lw	a1, 0(t0)
	jal	time2string
	
	# go back and do it all again
	j	timetemplate

	
# tick: update time pointed to by $a0
tick:	lw	t0, 0(a0)	# get time
	addi	t0, t0, 1	# increase
	andi	t1, t0, 0xf	# check lowest digit
	sltiu	t2, t1, 0xa	# if digit < a, okay
	bnez	t2, tiend
	addi	t0, t0, 0x6	# adjust lowest digit
	
	andi	t1, t0, 0xf0	# check next digit
	sltiu	t2, t1, 0x60	# if digit < 6, okay
	bnez	t2, tiend
	addi	t0, t0, 0xa0	# adjust digit
	
	li	t3, 0xF
	slli	t3, t3, 0x8
	and	t1, t0, t3	# check minute digit
	addi	t3, x0, 0xA
	slli	t3, t3, 0x8
	slt	t2, t1, t3	# if digit < a, okay
	bnez	t2, tiend
	addi	t0, t0, 0x600	# adjust digit - this one's okay, it's lower than 0x7FF 
	
	li	t3, 0xF
	slli	t3, t3, 0xC
	and	t1, t0, t3	# check last digit
	addi	t3, x0, 0x6
	slli	t3, t3, 0xC
	slt	t2, t1, t3	# if digit < 6, okay
	bnez	t2, tiend
	
	li	t3, 0xA
	slli	t3, t3, 0xC
	add	t0, t0, t3	# adjust last digit 
tiend:	sw	t0,0(a0)	# save updated result and saves 0x10000 to mytime
	jr	ra		# return

#########################################################
# Place for your functions: time2string, hex2asc, delay.#
#########################################################

hexasc:
	
  	andi a0, a0, 15 # maskerar till de sista fyra LSB
  	
  	addi a0, a0, 0x30
  	jr ra
  	

# delay

delay:
    ble a0, x0, done          # if (ms <= 0) return

while_loop:
    addi a0, a0, -1           # ms = ms - 1

    li   t0, 0                # i = 0
    li   t1, 10000              # gr ns f r i

for_loop:
    bge  t0, t1, end_for      # if (i >= 125) break
    addi t0, t0, 1            # i = i + 1
    j    for_loop             # g  tillbaka

end_for:
    bgt  a0, x0, while_loop   # while (ms > 0)

done:
    jr   ra                   # return
  
# time2string

time2string:
  PUSH ra
  PUSH s1
  
  addi s1,a0,0 # s1 = buffert pekare (timestr)
  
  
  # a0 = dit vi ska skriva
  # a1 = tid i BCD
  
  # f rsta nibble, minuter i tiotal
  srli a0, a1, 12
  jal hexasc
  
  sb a0,0(s1)
  
  # andra nibble
  srli a0, a1, 8
  jal hexasc
  
  sb a0,1(s1)
  
  # kolon
  li t0, 0x3A
  sb t0,2(s1)
  
  # tredje nibble
  srli a0, a1, 4
  jal hexasc
  
  sb a0,3(s1)
  
  # fj rde nibble
  addi a0,a1,0
  jal hexasc
  
  sb a0,4(s1)
  
  # null ending
  li t0, 0x00
  sb t0,5(s1)
  
  POP s1
  POP ra
  
  jr ra
  
  

