; PASS: cff
; Verify CFF handles both conditional branches and switch terminators.
; After flattening, the original branch/switch targets become switch cases.

; CHECK-LABEL: define i32 @branches_and_switch
; CHECK: cff.dispatch:
; CHECK: switch i32
; CHECK: store i32
; CHECK: ret i32

define i32 @branches_and_switch(i32 %x) {
entry:
  %cmp = icmp slt i32 %x, 10
  br i1 %cmp, label %low, label %check_mid

low:
  %a = mul i32 %x, 2
  br label %done

check_mid:
  switch i32 %x, label %fallback [
    i32 10, label %exact
    i32 20, label %double
  ]

exact:
  %b = add i32 %x, 100
  br label %done

double:
  %c = mul i32 %x, 3
  br label %done

fallback:
  %d = sub i32 %x, 1
  br label %done

done:
  %r = phi i32 [ %a, %low ], [ %b, %exact ], [ %c, %double ], [ %d, %fallback ]
  ret i32 %r
}
