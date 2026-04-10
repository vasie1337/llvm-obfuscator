; PASS: cff
; Verify that CFF inserts a dispatcher switch block.

; CHECK-LABEL: define i32 @multi_block
; CHECK: cff.dispatch:
; CHECK: switch i32
; CHECK: cff.default:

define i32 @multi_block(i32 %x) {
entry:
  %cmp = icmp sgt i32 %x, 0
  br i1 %cmp, label %then, label %else

then:
  %a = add i32 %x, 1
  br label %exit

else:
  %b = sub i32 %x, 1
  br label %exit

exit:
  %r = phi i32 [ %a, %then ], [ %b, %else ]
  ret i32 %r
}
