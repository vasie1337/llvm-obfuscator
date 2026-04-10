; PASS: bcf
; Verify BCF inserts the opaque predicate global and bogus blocks.

; CHECK: @.bcf_opaque = private global i32 0
; CHECK-LABEL: define i32 @bcf_target
; CHECK: .bogus
; CHECK: load {{.*}} @.bcf_opaque
; CHECK: icmp eq i32

define i32 @bcf_target(i32 %x) {
entry:
  %a = add i32 %x, 1
  %b = mul i32 %a, 2
  %c = sub i32 %b, %x
  ret i32 %c
}
