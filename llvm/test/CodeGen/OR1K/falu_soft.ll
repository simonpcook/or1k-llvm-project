; RUN: llc -march=or1k -float-abi=soft < %s | FileCheck %s

define float @f0(float %a, float %b) {
entry:
  %c = fadd float %a, %b
  ret float %c
}
; CHECK: f0:
; CHECK: l.jal __addsf3

define float @f1(float %a, float %b) {
entry:
  %c = fsub float %a, %b
  ret float %c
}
; CHECK: f1:
; CHECK: l.jal __subsf3

define float @f2(float %a, float %b) {
entry:
  %c = fmul float %a, %b
  ret float %c
}
; CHECK: f2:
; CHECK: l.jal __mulsf3

define float @f3(float %a, float %b) {
entry:
  %c = fdiv float %a, %b
  ret float %c
}
; CHECK: f3:
; CHECK: l.jal __divsf3

define float @f4(float %a, float %b) {
entry:
  %c = frem float %a, %b
  ret float %c
}
; CHECK: f4:
; CHECK: l.jal fmodf
