; PASS: instsub
; Verify that add is replaced with sub(a, neg(b)) pattern.

; CHECK-LABEL: define i32 @test_add
; CHECK-NOT: add i32 %a, %b
; CHECK: sub i32
; CHECK: ret i32

define i32 @test_add(i32 %a, i32 %b) {
  %r = add i32 %a, %b
  ret i32 %r
}
