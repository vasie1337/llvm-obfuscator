# IDA Hex-Rays pseudocode — Obfuscated (`demo_obfuscated.exe`)

Exported from IDA as HTML, converted to Markdown for GitHub rendering.

```c
int __fastcall main(int argc, const char **argv, const char **envp)
{
  void *v3; // rsp
  void *v4; // rsp
  void *v5; // rsp
  void *v6; // rsp
  void *v7; // rsp
  void *v8; // rsp
  void *v9; // rsp
  void *v10; // rsp
  _BYTE *v11; // rcx
  unsigned int v12; // eax
  _QWORD *v13; // rdx
  unsigned int *v14; // rcx
  int *v15; // rax
  int v16; // ecx
  int v17; // ecx
  void *v18; // r8
  int *v19; // rax
  int *v20; // rax
  _DWORD *v21; // rcx
  _QWORD *v22; // rdx
  int v23; // ecx
  __int64 v24; // rax
  int *v25; // rax
  int *v26; // rax
  int v27; // ecx
  unsigned int v28; // eax
  int *v29; // rax
  void *v31; // rsp
  void *v32; // rsp
  void *v33; // rsp
  void *v34; // rsp
  void *v35; // rsp
  void *v36; // rsp
  void *v37; // rsp
  void *v38; // rsp
  _BYTE *v39; // rcx
  unsigned int v40; // eax
  _QWORD *v41; // rdx
  unsigned int *v42; // rcx
  int *v43; // rax
  int v44; // ecx
  int v45; // ecx
  void *v46; // r8
  int *v47; // rax
  int *v48; // rax
  _DWORD *v49; // rcx
  _QWORD *v50; // rdx
  int v51; // ecx
  __int64 v52; // rax
  int *v53; // rax
  int *v54; // rax
  int v55; // ecx
  unsigned int v56; // eax
  int *v57; // rax
  _DWORD v58[8]; // [rsp-30h] [rbp-B0h] BYREF
  _BYTE v59[24]; // [rsp-10h] [rbp-90h] BYREF
  unsigned int v60; // [rsp+8h] [rbp-78h]
  unsigned int v61; // [rsp+Ch] [rbp-74h]
  __int64 v62; // [rsp+10h] [rbp-70h]
  __int64 v63; // [rsp+18h] [rbp-68h]
  __int64 v64; // [rsp+20h] [rbp-60h]
  unsigned int v65; // [rsp+28h] [rbp-58h]
  unsigned int v66; // [rsp+2Ch] [rbp-54h]
  __int64 v67; // [rsp+30h] [rbp-50h]
  int v68; // [rsp+3Ch] [rbp-44h]
  _DWORD *v69; // [rsp+40h] [rbp-40h]
  _BYTE *v70; // [rsp+48h] [rbp-38h]
  _QWORD *v71; // [rsp+50h] [rbp-30h]
  __int64 v72; // [rsp+58h] [rbp-28h]
  unsigned int *v73; // [rsp+60h] [rbp-20h]
  _QWORD *v74; // [rsp+68h] [rbp-18h]
  int *v75; // [rsp+70h] [rbp-10h]
  __int64 v76; // [rsp+78h] [rbp-8h]
  unsigned int *v77; // [rsp+80h] [rbp+0h]
  int *v78; // [rsp+88h] [rbp+8h]

  _main();
  while ( ((((_BYTE)dword_140010034 + 1) * (_BYTE)dword_140010034) & 1) != 0 )
  {
    v64 = 16;
    v31 = alloca(16);
    v32 = alloca(16);
    v63 = 32;
    v33 = alloca(32);
    v34 = alloca(16);
    v35 = alloca(32);
    v36 = alloca(16);
    v37 = alloca(16);
    v38 = alloca(16);
    v58[0] = 1003404076;
  }
  v76 = 16;
  v3 = alloca(16);
  v69 = v59;
  v4 = alloca(16);
  v70 = v59;
  v72 = 32;
  v5 = alloca(32);
  v71 = v58;
  v6 = alloca(16);
  v73 = v58;
  v7 = alloca(32);
  v74 = v58;
  v8 = alloca(16);
  v75 = v58;
  v9 = alloca(16);
  v77 = v58;
  v10 = alloca(16);
  v78 = v58;
  v58[0] = 1003404076;
  while ( 1 )
  {
    while ( ((((_BYTE)dword_140010034 + 1) * (_BYTE)dword_140010034) & 1) != 0 )
      ;
    v68 = *v78;
    if ( v68 == 855803118 )
      break;
    switch ( v68 )
    {
      case 922938880:
        while ( ((((_BYTE)dword_140010034 + 1) * (_BYTE)dword_140010034) & 1) != 0 )
        {
          v51 = 1174750953;
          if ( *v75 < dword_140010030 - 278435973 + 278435978 )
            v51 = 1877402589;
          *v78 = v51;
        }
        v23 = 1174750953;
        if ( *v75 < dword_140010030 - 278435973 + 278435978 )
          v23 = 1877402589;
        *v78 = v23;
        break;
      case 1003404076:
        while ( ((((_BYTE)dword_140010034 + 1) * (_BYTE)dword_140010034) & 1) != 0 )
        {
          v39 = v70;
          *v69 = 0;
          *(_DWORD *)v39 = _data_start__;
          *((_WORD *)v39 + 2) = word_14000C004;
          v39[6] = byte_14000C006;
          xor_decrypt(v39, 6, 71);
          printf(Format, v70);
          v40 = custom_hash(&unk_14000C01E);
          printf(&byte_14000C010, v40);
          v41 = v71;
          v42 = v73;
          v43 = v78;
          *v71 = &unk_14000C029;
          v41[1] = &unk_14000C03A;
          v41[2] = &unk_14000C04B;
          *v42 = 0;
          *v43 = 2123961343;
        }
        v11 = v70;
        *v69 = 0;
        *(_DWORD *)v11 = _data_start__;
        *((_WORD *)v11 + 2) = word_14000C004;
        v11[6] = byte_14000C006;
        xor_decrypt(v11, 6, 71);
        printf(Format, v70);
        v12 = custom_hash(&unk_14000C01E);
        printf(&byte_14000C010, v12);
        v13 = v71;
        v14 = v73;
        v15 = v78;
        *v71 = &unk_14000C029;
        v13[1] = &unk_14000C03A;
        v13[2] = &unk_14000C04B;
        *v14 = 0;
        *v15 = 2123961343;
        break;
      case 1174750953:
        while ( ((((_BYTE)dword_140010034 + 1) * (_BYTE)dword_140010034) & 1) != 0 )
        {
          v54 = v78;
          *v77 = 0;
          *v54 = 2120405162;
        }
        v26 = v78;
        *v77 = 0;
        *v26 = 2120405162;
        break;
      case 1457455917:
        while ( ((((_BYTE)dword_140010034 + 1) * (_BYTE)dword_140010034) & 1) != 0 )
        {
          v47 = v78;
          *v73 = _mm_cvtsi128_si32(
                   _mm_sub_epi32(
                     _mm_xor_si128(_mm_cvtsi32_si128(*v73), (__m128i)-1LL),
                     _mm_slli_epi32(_mm_cvtsi32_si128(~*v73), 1u)));
          *v47 = 2123961343;
        }
        v19 = v78;
        *v73 = _mm_cvtsi128_si32(
                 _mm_sub_epi32(
                   _mm_xor_si128(_mm_cvtsi32_si128(*v73), (__m128i)-1LL),
                   _mm_slli_epi32(_mm_cvtsi32_si128(~*v73), 1u)));
        *v19 = 2123961343;
        break;
      case 1749979882:
        while ( ((((_BYTE)dword_140010034 + 1) * (_BYTE)dword_140010034) & 1) != 0 )
        {
          v53 = v78;
          ++*v75;
          *v53 = 922938880;
        }
        v25 = v78;
        ++*v75;
        *v25 = 922938880;
        break;
      case 1877402589:
        while ( ((((_BYTE)dword_140010034 + 1) * (_BYTE)dword_140010034) & 1) != 0 )
        {
          v61 = *((_DWORD *)v74 + *v75);
          v52 = classify_score(*((unsigned int *)v74 + *v75));
          printf(&byte_14000C072, v61, v52);
          *v78 = 1749979882;
        }
        v66 = *((_DWORD *)v74 + *v75);
        v24 = classify_score(*((unsigned int *)v74 + *v75));
        printf(&byte_14000C072, v66, v24);
        *v78 = 1749979882;
        break;
      case 1972925853:
        while ( ((((_BYTE)dword_140010034 + 1) * (_BYTE)dword_140010034) & 1) != 0 )
        {
          v60 = *v77;
          v56 = fibonacci(*v77);
          printf(&byte_14000C082, v60, v56);
          *v78 = 1995971500;
        }
        v65 = *v77;
        v28 = fibonacci(*v77);
        printf(&byte_14000C082, v65, v28);
        *v78 = 1995971500;
        break;
      case 1995971500:
        while ( ((((_BYTE)dword_140010034 + 1) * (_BYTE)dword_140010034) & 1) != 0 )
        {
          v57 = v78;
          *v77 = _mm_cvtsi128_si32(_mm_sub_epi32(_mm_cvtsi32_si128(*v77), (__m128i)-1LL));
          *v57 = 2120405162;
        }
        v29 = v78;
        *v77 = _mm_cvtsi128_si32(_mm_sub_epi32(_mm_cvtsi32_si128(*v77), (__m128i)-1LL));
        *v29 = 2120405162;
        break;
      case 2120405162:
        while ( ((((_BYTE)dword_140010034 + 1) * (_BYTE)dword_140010034) & 1) != 0 )
        {
          v55 = 855803118;
          if ( (int)*v77 < dword_140010030 + 10 )
            v55 = 1972925853;
          *v78 = v55;
        }
        v27 = 855803118;
        if ( (int)*v77 < dword_140010030 + 10 )
          v27 = 1972925853;
        *v78 = v27;
        break;
      case 2123629579:
        while ( ((((_BYTE)dword_140010034 + 1) * (_BYTE)dword_140010034) & 1) != 0 )
        {
          v62 = v71[*v73];
          v45 = check_serial(v71[*v73]);
          v46 = &unk_14000C06A;
          if ( v45 )
            v46 = &unk_14000C064;
          printf(&byte_14000C051, v62, v46);
          *v78 = 1457455917;
        }
        v67 = v71[*v73];
        v17 = check_serial(v71[*v73]);
        v18 = &unk_14000C06A;
        if ( v17 )
          v18 = &unk_14000C064;
        printf(&byte_14000C051, v67, v18);
        *v78 = 1457455917;
        break;
      case 2123961343:
        while ( ((((_BYTE)dword_140010034 + 1) * (_BYTE)dword_140010034) & 1) != 0 )
        {
          v44 = 2140794733;
          if ( (int)*v73 < dword_140010030 - 113701803 + 113701806 )
            v44 = 2123629579;
          *v78 = v44;
        }
        v16 = 2140794733;
        if ( (int)*v73 < dword_140010030 - 113701803 + 113701806 )
          v16 = 2123629579;
        *v78 = v16;
        break;
      case 2140794733:
        while ( ((((_BYTE)dword_140010034 + 1) * (_BYTE)dword_140010034) & 1) != 0 )
        {
          v48 = v78;
          v49 = v75;
          v50 = v74;
          *v74 = 0x480000005FLL;
          v50[1] = 0x1F00000037LL;
          *((_DWORD *)v50 + 4) = 10;
          *v49 = 0;
          *v48 = 922938880;
        }
        v20 = v78;
        v21 = v75;
        v22 = v74;
        *v74 = 0x480000005FLL;
        v22[1] = 0x1F00000037LL;
        *((_DWORD *)v22 + 4) = 10;
        *v21 = 0;
        *v20 = 922938880;
        break;
    }
  }
  return 0;
}
```
