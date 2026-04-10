; PASS: instsub
; Verify that sub is replaced with add(a, neg(b)) pattern.

; CHECK-LABEL: define i32 @test_sub
; CHECK-NOT: sub i32 %a, %b
; CHECK: add i32
; CHECK: ret i32

define i32 @test_sub(i32 %a, i32 %b) {
  %r = sub i32 %a, %b
  ret i32 %r
}
