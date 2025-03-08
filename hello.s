  .text
  .globl main
main:
  addi sp, sp, -64
  sw ra, 60(sp)
entry:
  li t0, 0
  li t1, 1
  add t2, t0, t1
  sw t2, 0(sp)
  li t0, 0
  li t1, 2
  add t2, t0, t1
  sw t2, 4(sp)
  lw t0, 0(sp)
  lw t1, 4(sp)
  add t2, t0, t1
  sw t2, 8(sp)
  li t0, 0
  li t1, 1
  add t2, t0, t1
  sw t2, 12(sp)
  li t0, 0
  li t1, 2
  add t2, t0, t1
  sw t2, 16(sp)
  li t0, 0
  li t1, 3
  add t2, t0, t1
  sw t2, 20(sp)
  li t0, 0
  li t1, 4
  add t2, t0, t1
  sw t2, 24(sp)
  lw t0, 20(sp)
  lw t1, 24(sp)
  mul t2, t0, t1
  sw t2, 28(sp)
  lw t0, 16(sp)
  lw t1, 28(sp)
  add t2, t0, t1
  sw t2, 32(sp)
  li t0, 0
  lw t1, 32(sp)
  sub t2, t0, t1
  seqz t2, t2
  sw t2, 36(sp)
  lw t0, 12(sp)
  lw t1, 36(sp)
  add t2, t0, t1
  sw t2, 40(sp)
  lw t0, 8(sp)
  lw t1, 40(sp)
  sub t2, t0, t1
  seqz t2, t2
  sw t2, 44(sp)
  lw a0, 44(sp)
  lw ra, 60(sp)
  addi sp, sp, 64
  ret
