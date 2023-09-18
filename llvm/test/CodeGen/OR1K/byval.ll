; RUN: llc -march=or1k < %s | FileCheck %s
%struct.s = type { [32 x i32] }

define void @f1(ptr byval(%struct.s) %s) {
entry:
  call void @f0(ptr byval(%struct.s) %s)
  ret void
}

declare void @f0(ptr byval(%struct.s))
; CHECK: f1:
; CHECK: l.addi  r1, r1, -136
; CHECK: l.addi  r5, r0, 128
; CHECK: l.jal   memcpy
