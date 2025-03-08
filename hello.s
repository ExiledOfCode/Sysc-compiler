  .text
  .globl main
main:
  li t0, 0
  li t1, 1
  add a0, t0, t1
  li t0, 0
  li t1, 2
  add a0, t0, t1
  li t0, 0
  li t1, 1
  add a0, t0, t1
  mv t0, a0
  li t0, 0
  li t1, 2
  add a0, t0, t1
  mv t1, a0
  add a0, t0, t1
  ret
