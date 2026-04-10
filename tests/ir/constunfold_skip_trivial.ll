; PASS: constunfold
; Constants 0, 1, -1 should NOT be unfolded.

; CHECK-LABEL: define i32 @test_trivial
; CHECK: add i32 %a, 0
; CHECK: sub i32 %b, 1
; CHECK-NOT: load volatile
; CHECK: ret i32

define i32 @test_trivial(i32 %a, i32 %b) {
  %x = add i32 %a, 0
  %y = sub i32 %b, 1
  %r = xor i32 %x, %y
  ret i32 %r
}
