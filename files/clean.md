# IDA Hex-Rays pseudocode — Clean (`demo_clean.exe`)

Exported from IDA as HTML, converted to Markdown for GitHub rendering.

```c
int __fastcall main(int argc, const char **argv, const char **envp)
{
  int v3; // eax
  int v4; // ecx
  const char *v5; // r8
  const char *v6; // rax
  int v7; // eax
  unsigned int v9; // [rsp+2Ch] [rbp-54h]
  const char *v10; // [rsp+30h] [rbp-50h]
  int k; // [rsp+38h] [rbp-48h]
  int j; // [rsp+3Ch] [rbp-44h]
  _QWORD v13[2]; // [rsp+40h] [rbp-40h]
  int v14; // [rsp+50h] [rbp-30h]
  int i; // [rsp+5Ch] [rbp-24h]
  _QWORD v16[4]; // [rsp+60h] [rbp-20h]
  _BYTE v17[11]; // [rsp+85h] [rbp+5h] BYREF

  _main();
  *(_DWORD *)&v17[7] = 0;
  strcpy(v17, "/\"++(f");
  xor_decrypt(v17, 6, 71);
  printf("msg: %s\n", v17);
  v3 = custom_hash("obfuscated");
  printf("hash: 0x%08x\n", v3);
  v16[0] = "ABCD1234EFGH5678";
  v16[1] = "0000000000000000";
  v16[2] = "short";
  for ( i = 0; i < 3; ++i )
  {
    v10 = (const char *)v16[i];
    v4 = check_serial(v10);
    v5 = "INVALID";
    if ( v4 )
      v5 = "VALID";
    printf("serial '%s' -> %s\n", v10, v5);
  }
  v13[0] = 0x480000005FLL;
  v13[1] = 0x1F00000037LL;
  v14 = 10;
  for ( j = 0; j < 5; ++j )
  {
    v9 = *((_DWORD *)v13 + j);
    v6 = (const char *)classify_score(v9);
    printf("score %d -> %s\n", v9, v6);
  }
  for ( k = 0; k < 10; ++k )
  {
    v7 = fibonacci((unsigned int)k);
    printf("fib(%d) = %d\n", k, v7);
  }
  return 0;
}
```
