; PASS: simd
; Verify that i8 operations are not promoted (only i32/i64).

; CHECK-LABEL: define i8 @test_small
; CHECK: add i8 %a, %b
; CHECK-NOT: insertelement
; CHECK: ret i8

define i8 @test_small(i8 %a, i8 %b) {
  %r = add i8 %a, %b
  ret i8 %r
}
