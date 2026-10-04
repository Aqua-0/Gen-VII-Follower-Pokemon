.syntax unified
.arm
.section .text.plugin_compat, "ax", %progbits
.align 2

.global Follower3gx_MapProcessMemoryExV1
.type Follower3gx_MapProcessMemoryExV1, %function
Follower3gx_MapProcessMemoryExV1:
  push {r4, lr}
  ldr r4, [sp, #8]
  svc 0xa0
  pop {r4, pc}
.size Follower3gx_MapProcessMemoryExV1, . - Follower3gx_MapProcessMemoryExV1

.global Follower3gx_SetProcessMemoryRwx
.type Follower3gx_SetProcessMemoryRwx, %function
Follower3gx_SetProcessMemoryRwx:
  mov r1, #1
  mov r2, #0
  mov r3, #0
  svc 0xb3
  bx lr
.size Follower3gx_SetProcessMemoryRwx, . - Follower3gx_SetProcessMemoryRwx

.global Follower3gx_RefreshAndCheckUserWriteTlb
.type Follower3gx_RefreshAndCheckUserWriteTlb, %function
Follower3gx_RefreshAndCheckUserWriteTlb:
  mov r3, #0
  mcr p15, 0, r3, c7, c10, 4
  mcr p15, 0, r3, c8, c7, 0
  mcr p15, 0, r3, c7, c5, 4

  mcr p15, 0, r0, c7, c8, 3
  mrc p15, 0, r3, c7, c4, 0
  tst r3, #1
  bne 1f
  mcr p15, 0, r1, c7, c8, 3
  mrc p15, 0, r3, c7, c4, 0
  tst r3, #1
  bne 1f
  mcr p15, 0, r2, c7, c8, 3
  mrc p15, 0, r3, c7, c4, 0
  tst r3, #1
  bne 1f

  mov r0, #1
  bx lr
1:
  mov r0, #0
  bx lr
.size Follower3gx_RefreshAndCheckUserWriteTlb, . - Follower3gx_RefreshAndCheckUserWriteTlb

.section .note.GNU-stack, "", %progbits
