; PASS: bbfission
; A function with only one block and few instructions should be left alone.

; CHECK-LABEL: define i32 @test_small
; CHECK-NOT: bbf.junk
; CHECK: ret i32

define i32 @test_small(i32 %a, i32 %b) {
  %r = add i32 %a, %b
  ret i32 %r
}
