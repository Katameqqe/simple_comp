# simple_comp
So its simple compression program implemented by linear Huffman algorithm.
Its work only for number of unique bytes in text <= 128, so we can compress it from 8 to 7 bits per byte 
For Example we are having list of unique bytes in text
```
> 0x41 0x42 0x43 0x44
```
And we want to build a dictionary that we will be using to compress data in text
```
> 0x41: 0100 0001 -> 00
> 0x42: 0100 0010 -> 01
> 0x43: 0100 0011 -> 10
> 0x44: 0100 0100 -> 11
```
So our dictionary is 
```
> 00 01 10 11
```
and if we have text like this
```
ABCDABCDBBCCDDBBCCDDAADDAA
```
we will compress it like that
```
A BCDABCD
A -> 00

Compress data: 00
```
```
A B CDABCD
B -> 01

Compress data: 00.01
```
```
AB C DABCD
C -> 10

Compress data: 00.01.10
```
```
ABC D ABCD
D -> 11

Compress data: 00.01.10.11
```
and so on. We also adding 3 bit at the start of compress data to tell how many bits **R** at the last byte we don't need to read
```
                R        Compres data
Compress data: 001 + 00011 011000110110001 + 0
Compress data: 10000011 01100011 01100010 
```
So compressed file will look like that
```
Original data:
ABCDABCDAB
41 42 43 44 41 42 43 44 41 42

Length   Dictionary    Data
  04    41 42 43 44  83 63 62
```



