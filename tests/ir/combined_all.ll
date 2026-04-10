; PASS: function(instsub,mbasub,cff,bcf),strenc
; All passes composed together on a non-trivial function with strings.

; Instruction substitution + MBA should eliminate plain add/sub:
; CHECK-NOT: add i32 %x, 1
; CHECK-NOT: sub i32 %x, 1

; CFF should insert a dispatcher:
; CHECK: cff.dispatch

; BCF should insert opaque predicate infrastructure:
; CHECK: @.bcf_opaque

; String encryption should create the decryption ctor:
; CHECK: @__strenc_ctor

@.str_pos = private constant [9 x i8] c"positive\00"
@.str_neg = private constant [9 x i8] c"negative\00"

declare i32 @puts(ptr)

define i32 @combined(i32 %x) {
entry:
  %cmp = icmp sgt i32 %x, 0
  br i1 %cmp, label %pos, label %neg

pos:
  %a = add i32 %x, 1
  call i32 @puts(ptr @.str_pos)
  br label %done

neg:
  %b = sub i32 %x, 1
  call i32 @puts(ptr @.str_neg)
  br label %done

done:
  %r = phi i32 [ %a, %pos ], [ %b, %neg ]
  ret i32 %r
}
