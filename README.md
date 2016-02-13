# BOLGEFUCK SPECIFICATION

##### Memory

1. Tape: An unbounded array of bytes indexed from 0 to infinity, each cell initialized to 0. All code and data are stored here. 
2. Cryptor: An unbounded array of bytes indexed from 0 to infinity, each cell initialized to 0. Used for obfuscation purposes only. 

##### Registers

1. CP - Code pointer. An integer of unbounded magnitude, initialized to 0. The byte at tape[CP] is executed at each iteration. 
2. DP - Data pointer. An integer of unbounded magnitude, initialized to 0. The byte at tape[DP] is operated on by some of the instructions. 
3. EP - Cryptor pointer. An integer of unbounded magnitude, initialized to 0. Used for obfuscation purposes only. 

##### Operation

1. The bolgefuck file is read in to the tape, overwriting the contents of the tape with the contents of the file. 
2. The cryptor may be altered to a known value by the interpreter, this should not change between runs or between programs. 
3. If the first 8 bytes of the tape reads "wimpmode", then wimpmode is turned on. Otherwise wimpmode is off. 
4. The main loop is entered. 

##### Main loop

1. If CP or DP is negative, set it to 0. 
2. If wimpmode is off, encrypt the byte at tape[CP]. Otherwise skip to 3. 
3. If the byte at tape[CP] is a valid instruction, execute it. Otherwise, if wimpmode is on, terminate execution with error, otherwise encrypt the byte at tape[CP]. 
4. Goto 1. 

##### Instructions

1. H means terminate execution. 
2. > means increment DP. Increment CP. 
3. < means decrement DP. Increment CP. 
4. i means get 1 byte from stdin and write it to tape[DP]. Increment CP. 
5. p means print byte at tape[DP]. Increment CP. 
6. s means write tape[CP+1] to tape[DP]. Increment CP by 2. 
7. J a X Y means if tape[DP]==a CP += X else CP += Y. Example: `Jkb+1011*-1*` means if tape[DP]=='k' CP+=11 else CP--. The format is as follows: b or h signifies binary or hex. + or - signifies positive or negative. Followed by the number in binary or hex. Both numbers must be in the same format (binary or hex). Each number is followed by a * to indicate the number has ended. 

The encryption routine makes a string out of the contents of the tape up to the furthest point ever reached by either CP or DP, and then appends the contents of cryptor (also up to the furthest point reached by EP) to that string. A SHA256 hash is computed for this string, and then a byte of the resulting hash is returned, the location of that byte in the hash is the sum of the hash % 32. EP is then incremented and the returned byte is written to Cryptor[EP]. The new Cryptor is appended to the string that was hashed, the string is hashed and the byte returned by the hash is used to overwrite the byte at tape[CP]. 


## Building

      make

That's right, all you have to do is cd into the directory which contains the makefile and type make. 

## How to use

      ./bf program

If you would like to pipe some input from stdin into your program:

      echo 111111-11= | ./bf sub

### Miscellaneous trivia

My implementation uses long long (64 byte signed integer) types. This implementation is present throughout the code. Other languages that use unbounded integer types may be more suited for implementation of the specification. Also note a lot of things in the code depend on the TAPE_MAX_SIZE constant. Be sure to keep this constant much lower than your system memory since various functions in the code (especially the encryption function) can use a lot more memory than constant suggests. I used regex and stoll for cba2n because I wanted to keep the code short and readable, though I could code my own finite state machine and atoi. 

The idea of the cryptor is to prevent cycles, since with each hash the cryptor will be changed. This is clear in the specification but since the size of cryptor is limited in the implementation there will still be cycles (though whether if these cycles would have practical impact on the obfuscation I don't know). 

You may have to press enter after typing in input. You can change this by changing your terminal settings, it's not a feature of the program. 

You may have noticed that all of the programs provided as examples use wimpmode. This is because I have no idea how to write programs that work without turning wimpmode on. Please tell me if you manage to write a hello world program without using wimpmode. 

##### Random remarks

This language is mostly based on brainfuck. In fact most of the operators are directly copied from it. The major difference is the use of a parametered jump rather than brackets. This was intended to save one instruction but also to make the wimpmode easier to use. I spent a good 10-40 hours on this, mostly reading about other esolanguages (brainfuck, malbolge and bitbitjump), turing completeness, learning C++ (I've never programmed in C++ before, a fact you may deduce from reading the source code), writing and debugging my own programs in bolgefuck, and of course designing and implementing the language itself. 

The intent behind this program is to make something harder to program in than malbolge, and still be Turing complete. I'm pretty sure the wimpmode is Turing complete since the wimpmode is so similar to brainfuck (and Turing machines) although I don't have a proof. The central part that's supposed to make it harder is the use of SHA256 hash over the entire program rather than having a pre-defined which-byte-encrypts-to-which-byte function. My hope is that by doing this, it makes an analytical approach impossible unless SHA256 is broken and programmers would have to use brute force search to find a working program. I don't know how effective brute force search would be because I haven't tried it. It might be the case that the first hello world program would require more memory than can be addressed by the types used in the implementation presented here in which case someone will have to write their own implementation to remove the limits, which I would like to see. 

I originally planned for the encryption function to encrypt every single byte in the tape up to the furthest position ever reached by DP or CP, but I didn't since I didn't think there will be a hello world program written in the current version of the language anytime soon. I may add this feature in future versions of the language if the current version proves to be too easy to use. 

