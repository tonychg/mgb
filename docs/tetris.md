Debug SM83 using tetris ROM (World)

```bash
./build/sm83-debugger <rom>
goto 0233
set FF44 94
goto 02c2
```

After the previous operations
```
00:02C0 E0 4B -> LDH a8[$4B] A
     Z = 1 | N = 0
     H = 0 | C = 0
  A = $00  |  F = $80
  00000000 | 10000000
  B = $00  |  C = $00
  00000000 | 00000000
  D = $00  |  E = $D8
  00000000 | 11011000
  H = $97  |  L = $FF
  10010111 | 11111111
      SP = $CFFF
   1100111111111111
      PC = $02C3
   0000001011000011
  IME = 1  | HALT = 0
  DIV = 0  | TIMA = 0
  M-cycles = 229110 | T-cycles = 916440
00:02C2 E0 06 -> LDH a8[$06] A
```
