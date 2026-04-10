; PASS: constunfold
; Verify that the integer constant 42 is replaced with a volatile-load
; based computation (the literal 42 should no longer appear as a direct
; operand of the add).

; CHECK-LABEL: define i32 @test_const
; CHECK: load volatile i32, ptr @.constunfold_32
; CHECK-NOT: add i32 %a, 42
; CHECK: ret i32

define i32 @test_const(i32 %a) {
  %r = add i32 %a, 42
  ret i32 %r
}
