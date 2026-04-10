; PASS: bbfission
; Verify that the pass splits blocks and introduces junk blocks + opaque branches.

; CHECK-LABEL: define i32 @test_fission
; CHECK: bbf.frag
; CHECK: bbf.junk
; CHECK: ret i32

define i32 @test_fission(i32 %a, i32 %b) {
entry:
  %x = add i32 %a, %b
  %y = mul i32 %x, 3
  %z = sub i32 %y, %a
  %w = add i32 %z, 7
  %v = xor i32 %w, %b
  %r = add i32 %v, 1
  ret i32 %r
}
