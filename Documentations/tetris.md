# Debug SM83 using tetris ROM (World)

## Some useful addresses

```
# Select Players Screen
break 0x03ae 
break 0x041c

# Demo range
break 0x041f
break 0x0471

# Display Type-A Level Select / Top Score screen
break 0x157b
break 0x15b9
```

## Simulate inputs

```
# Set a breakpoint before read
break 29ce
# A
0b11011110
set ff00 0xde
set ff0f 0x10
# B
0b11011101
set ff00 0xdd
set ff0f 0x10
# Select
0b11011011
set ff00 0xdb
set ff0f 0x10
# Start
0b11010111
set ff00 0xd7
set ff0f 0x10
# Right
0b11101110
set ff00 0xee
set ff0f 0x10
# Left
0b11101101
set ff00 0xed
set ff0f 0x10
# Up
0b11101011
set ff00 0xeb
set ff0f 0x10
# Down
0b11100111
set ff00 0xe7
set ff0f 0x10
```
