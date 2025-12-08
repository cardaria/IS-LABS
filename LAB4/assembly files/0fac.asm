    addi t0,zero,3               # set t0 to input value (n)
    addi t1,zero,1               # i = 1      (inner counter)
    addi t2,zero,1               # result = 1
    addi tp,t0,1                 # tp = n + 1 (loop limit)

loop_outer:
    beq  t1,tp,done              # if i == n+1 -> done
    add  sp,zero,zero            # sp = 0 (temporary product)
    add  gp,t1,zero              # gp = i (inner counter)

loop_inner:
    beq  gp,zero,mult_done        # if gp == 0 -> inner loop done
    add  sp,sp,t2                 # sp += t2
    addi gp,gp,-1               # gp--
    beq  zero,zero,loop_inner    # unconditional jump

mult_done:
    add  t2,sp,zero               # t2 = sp (update result)
    addi t1,t1,1                  # i++
    beq  zero,zero,loop_outer    # unconditional jump

done:
    beq  zero,zero,done           # stop loop (always true, branches to itself)