  .text
  .globl main
main:
  addi sp, sp, -48
  sw ra, 44(sp)
entry:
  li t0, 0
  li t1, 4
  add t2, t0, t1
  sw t2, 0(sp)
  li t0, 0
  li t1, 3
  add t2, t0, t1
  sw t2, 4(sp)
  lw t0, 0(sp)
  lw t1, 4(sp)
  mul t2, t0, t1
  sw t2, 8(sp)
  li t0, 0
  li t1, 1
  add t2, t0, t1
  sw t2, 12(sp)
  li t0, 0
  li t1, 2
  add t2, t0, t1
  sw t2, 16(sp)
  lw t0, 12(sp)
  lw t1, 16(sp)
  add t2, t0, t1
  sw t2, 20(sp)
  li t0, 0
  lw t1, 20(sp)
  sub t2, t0, t1
  seqz t2, t2
  sw t2, 24(sp)
  lw t0, 8(sp)
  lw t1, 24(sp)
  sub t2, t0, t1
  seqz t2, t2
  sw t2, 28(sp)
  lw a0, 28(sp)
  lw ra, 44(sp)
  addi sp, sp, 48
  ret
