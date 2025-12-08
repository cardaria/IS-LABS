.text
	addi t0,zero,4     # set t0 to 4
	addi t2,t0,2	  # test addi, set t2
	add  t2,t2,t0  # start of counter. Should be 10
loop:	beq  t2,t0,done # test, jump to done. 
        addi t2,t2,-3   # decrement. Loops twice.
        beq  zero,zero,loop   # emulating an unconditional jump
done:	add  zero,zero,zero	  # NOP	
	
