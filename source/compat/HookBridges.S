.syntax unified
.arm
.section .text.field_hook_bridges, "ax", %progbits
.align 2

.extern ctrpfHook__ExecuteOriginalFunction
.extern Follower3gx_PreUpdate
.extern Follower3gx_PostUpdate
.extern Follower3gx_HostTerminate
.extern g_Follower3gxUpdateExpectedReturn
.extern g_Follower3gxTerminateExpectedReturn

.global Follower3gx_UpdateHookBridge
.type Follower3gx_UpdateHookBridge, %function
Follower3gx_UpdateHookBridge:
  push {r4-r8, lr}
  mov r4, r0
  mov r5, lr
  mrc p15, 0, r8, c13, c0, 3
  ldr r8, [r8, #0x3c]
  ldr r6, =g_Follower3gxUpdateExpectedReturn
  ldr r6, [r6]
  cmp r5, r6
  bne 1f

  mov r0, r4
  bl Follower3gx_PreUpdate
  tst r0, #1
  movne r0, #0
  bne 3f

1:
  mrc p15, 0, r0, c13, c0, 3
  str r8, [r0, #0x3c]
  mov r0, r4
  bl ctrpfHook__ExecuteOriginalFunction
  mov r7, r0
  cmp r5, r6
  bne 2f
  cmp r7, #0
  bne 2f
  mov r0, r4
  bl Follower3gx_PostUpdate
2:
  mov r0, r7
3:
  pop {r4-r8, pc}
.size Follower3gx_UpdateHookBridge, . - Follower3gx_UpdateHookBridge

.global Follower3gx_TerminateHookBridge
.type Follower3gx_TerminateHookBridge, %function
Follower3gx_TerminateHookBridge:
  push {r4-r8, lr}
  mov r4, r0
  mov r5, lr
  mrc p15, 0, r8, c13, c0, 3
  ldr r8, [r8, #0x3c]
  ldr r6, =g_Follower3gxTerminateExpectedReturn
  ldr r6, [r6]
  cmp r5, r6
  bne 4f

  mov r0, r4
  bl Follower3gx_HostTerminate
  cmp r0, #0
  beq 5f

4:
  mrc p15, 0, r0, c13, c0, 3
  str r8, [r0, #0x3c]
  mov r0, r4
  bl ctrpfHook__ExecuteOriginalFunction
  b 6f
5:
  mov r0, #0
6:
  pop {r4-r8, pc}
.size Follower3gx_TerminateHookBridge, . - Follower3gx_TerminateHookBridge

.ltorg
.section .note.GNU-stack, "", %progbits
