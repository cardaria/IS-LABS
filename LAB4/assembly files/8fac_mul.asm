    addi t0,zero,8               # set t0 to input value (n)
    addi t1,zero,1               # i = 1      (counter)
    addi t2,zero,1               # result = 1
    addi tp,t0,1                 # tp = n + 1 (loop limit)

loop_outer:
    beq  t1,tp,done              # if i == n+1 -> done
    mul  t2,t2,t1                # t2 = t2 * i
    addi t1,t1,1                 # i++
    beq  zero,zero,loop_outer    # unconditional jump

done:
    beq  zero,zero,done          # stop loop (always true, branches to itself)
