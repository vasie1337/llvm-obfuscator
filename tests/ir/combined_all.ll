; PASS: function(instsub,mbasub,simd,constunfold,cff,bcf),strenc
; Same pipeline order as clang -fpass-plugin default late EP (PassPlugin.cpp).

; Volatile const pool + SIMD + flattening + bogus branches + strings:
; CHECK: @.constunfold_32
; Instruction substitution + MBA should eliminate plain add/sub:
; CHECK-NOT: add i32 %x, 1
; CHECK-NOT: sub i32 %x, 1

; CFF should insert a dispatcher:
; CHECK: cff.dispatch

; SIMD obfuscation (i32 adds promoted to vector ops):
; CHECK: insertelement <4 x i32>

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
