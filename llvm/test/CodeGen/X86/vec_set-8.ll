; NOTE: Assertions have been autogenerated by utils/update_llc_test_checks.py
; RUN: llc < %s -mtriple=i386-unknown -mattr=+sse4.2 | FileCheck %s --check-prefix=X86
; RUN: llc < %s -mtriple=x86_64-unknown -mattr=+sse4.2 | FileCheck %s --check-prefix=X64

define <2 x i64> @test(i64 %i) nounwind  {
; X86-LABEL: test:
; X86:       # BB#0:
; X86-NEXT:    movsd {{.*#+}} xmm0 = mem[0],zero
; X86-NEXT:    retl
;
; X64-LABEL: test:
; X64:       # BB#0:
; X64-NEXT:    movq %rdi, %xmm0
; X64-NEXT:    retq
  %tmp10 = insertelement <2 x i64> undef, i64 %i, i32 0
  %tmp11 = insertelement <2 x i64> %tmp10, i64 0, i32 1
  ret <2 x i64> %tmp11
}
