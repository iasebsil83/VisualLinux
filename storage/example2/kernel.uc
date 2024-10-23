; -------- TOOLS --------

; kernel => user equivalent
L backToProgExec
O r0000
J r0000




; -------- SYSCALLS --------

; declarations: types
D w0000 SYSCALL_PROC
D w0001 SYSCALL_READ
D w0002 SYSCALL_WRITE
D w0003 SYSCALL_OPEN
D w0004 SYSCALL_CLOSE
D w0005 SYSCALL_EXIT

; declarations: return values
D wffff SYSCALL_UNDEFINED

; execution
L syscalls
E dSYSCALL_PROC
J lsyscall_proc
E dSYSCALL_READ
J lsyscall_read
E dSYSCALL_WRITE
J lsyscall_write
E dSYSCALL_OPEN
J lsyscall_open
E dSYSCALL_CLOSE
J lsyscall_close
E dSYSCALL_EXIT
J lsyscall_exit
; no syscall found with that ID
W dSYSCALL_UNDEFINED
J lbackToProgExec

; syscalls : IO
L syscall_open
; do stuff
J lbackToProgExec

L syscall_close
; do stuff
J lbackToProgExec

L syscall_read
; do stuff
J lbackToProgExec

L syscall_write
; do stuff
J lbackToProgExec





; -------- PROCESSES --------

; declarations: process table

; execution
