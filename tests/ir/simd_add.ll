; PASS: simd
; Verify that scalar add i32 is promoted to vector operations.

; CHECK-LABEL: define i32 @test_add
; CHECK-NOT: add i32 %a, %b
; CHECK: insertelement <4 x i32>
; CHECK: add <4 x i32>
; CHECK: shufflevector
; CHECK: extractelement
; CHECK: ret i32

define i32 @test_add(i32 %a, i32 %b) {
  %r = add i32 %a, %b
  ret i32 %r
}
