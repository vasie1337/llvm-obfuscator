; PASS: instsub
; Verify that xor is replaced with (a|b) & ~(a&b) pattern.

; CHECK-LABEL: define i32 @test_xor
; CHECK-NOT: xor i32 %a, %b
; CHECK: or i32
; CHECK: and i32
; CHECK: ret i32

define i32 @test_xor(i32 %a, i32 %b) {
  %r = xor i32 %a, %b
  ret i32 %r
}
