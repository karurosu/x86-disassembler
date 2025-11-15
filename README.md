# README #

Note: This repo was recovered from a backup as the original was lost, unfortunately commit history is lost.

Technologies: C, Apache Portable Runtime

Status: incomplete.

This is an x86 disassembler written in C that can process ELF files and run in windows, linux and OSX. It works using a data centric approach and was implemented using "pseudo-objects".

# What works #
- Main flow
- 'Pluggable' binary parser architecture
- ELF parser (based on libELF)
- pretty printer
- opcode database
- the opcode decoder (1,2 and 3 bytes)
- Base operand parsers
- 'Z' operand parser.

# The objective #

This is a project turned in for my masters degree, it is an x86 disassembler that had to met the following:

- Completely written in C (no C++ allowed)
- Must compile and run in Windows and POSIX OS (mainly windows and linux)
- Parse both ELF and winPE files

Additionally I added the following goals (mostly because I wanted to):

- Compile and run in OSX
- Simple to implement (I didn't have much time)
- Object oriented-like implementation

# More Information #

From the beginning I knew this was a pretty difficult problem to tackle, not because its technical complexity (it is complicated, but everything is pretty well documented), but because x86 is one of the most complex architectures, and I knew for a fact that it would require a LOT of code. To add some difficulty, we had to design the architecture in some way that the same code would run in both windows and linux and be able to parse binaries for both OS.

I divided the work into 4 main 'problems' to solve: how to disassemble, how to parse binary file, how to make it modular and how to make it work on both OS (plus OSX, because is _mostly_ linux and my dev system is a mac). In the next sections I'll describe each problem and how I solved it. They are not in chronological order, I usually solved a bit of one problem that enabled me to solve part of another and jumped from one to the other, I'll try to put details of the order if I remember or has some significance. 

## The disassembler ##

The very first thing I did before any code was to disassemble a binary. I armed myself with the trusty x86 programmers guide(s), an existing disassembler, a hex editor and a test program (hello world of course). And I set myself to disassemble by hand. Basically, I took the hex output of the code section and tried to replicate the disassembled output on a piece of paper, I managed to get my first instruction after 3-4 hours of reading and making sense of the tables.

The next step was to try to find some common ground, that's when I learned about the opcode decoding (using a table) and operand decoding (using a table and a lot of pages). At this point I found something that would change the design: the amazing x86 reference XML ([here](http://ref.x86asm.net)). This had everything I needed, from the opcodes, mnemonics, opcode family and more importantly the "operand parser types".

So rather than writing a big "if" or decision tree, I decided to use a database approach to decoding, I implemented a database that contained all the information from the XML in a "C friendly" structure and used searches on the database to decode. You can find the details on the opcodedb project ([here](https://github.com/karurosu/x86-opcode-db-gen)). With this, I was able to decode any opcode instantly (ok, not instantly but O(n) where n is the byte size, and 1 <= n <= 3) and then have any reference I needed for decoding the operands.

So, to explain: the database allowed me to take 1-3 bytes of the opcode and obtain one or more 'Entries' which described the mnemonic and the 'operand type', using this information I could pass all the data to an 'operand parser' which extracted the actual parameters passed to the opcode. This also had the side effect of moving the file pointer to the next opcode.

I did not give much thought to operand decoding, there are basically 33 different ways of parsing operands (named from A-Z, plus SSE, rAX and similar, you can find more info on the x86 XML), I implemented one ('Z') which corresponds to the BSWAP instruction family (eax, ecx, etc) mainly because it was a fixed length opcode (2 bytes!). I never got around to implement the rest, I left that to the end because it was pretty much translating the decoding mechanism of each method from the manual to C (something that is mechanical and could be 'crunched' if needed).

Of course, before I could even implement any of the above I had some other things to solve...

## Parsing the binary file ##
The objective was to parse 2 different binary types: ELF and WinPE, I found a nice library for parsing ELF files (libELF) so I decided to use that one.

From reading both file formats I decided to use an abstraction in the form of files and sections. A parser would take a file and return a series of 'section' structs with pointers to the code blob (if it existed). Using libELF I built a parser that obtained each section and created the respective structs.

As a final touch, I created some generic functions for reading binary blobs, so I could do things like: section->readInt() and get a nicely formatted int regardless of how the binary was parsed.

## Making it modular ##

I was restricted to pure C, and while I could design something around functions and structs I wanted to do something different. At that time I was working with the UEFI framework, so I decided to copy one of the features. At the risk of oversimplifying, UEFI has a concept of 'objects' called drivers and protocols, each one is a struct that contains all its own data, as well as pointers to each one of the functions that it implements. In practice it means that if you get a Video driver 'instance', you can call 'methods' with a syntax like: VideoDriver->Draw(VideoDriver, stuff), and that kinda looks like objects.

So I replicated said idea and created structures for each of my 'classes', each contained 'properties' (data members of the struct) as well as 'methods' (functional pointers). To implement a class I defined the struct and then a type for each method. For example, to have a foo method I would define a functional pointer type classFooMethodType and created a member of the structure called foo of type classFooMethodType. In order to work, each method had to receive a pointer to an struct of the class (which, in pure python fashion I called self).

In practice, if you had an 'object' x, you could call method foo by writing: x->foo(x) and in the 'method' you could use self->property to access the object.

The end result worked fairly well, it allowed me to integrate reusable objects and design a simple OOP architecture. The main drawback is that creating the actual instances was overly complicated and cumbersome. I ended using 'singletons' for most of the classes, and reserved dynamic allocation to those that really deserved them (like the section instances). The creation process could probably been simplified by specifying a constructor mechanism (which I am fairly sure UEFI has), but I did not have time for that, and it would add zero benefit to the program at this stage.

In retrospective, it was a fun experiment, but I would not do something like that again unless I had a very specific reason (like building an extensible framework like the UEFI guys did). Although, coding by using self->something in C was pretty cool...

## Making it work on multiple OS ##

For most basic things there is functionality that is common across all OSs, but there are some things which can get tricky (especially low level OS things). For most of the design I stuck with the C standard libraries, but here may be situations where that was not possible.

At some point in my life I made a simple compatibility layer that exposed an unified API for windows and linux, this time I didn't have the time to code something like that, so I decided to use a trusted library: the Apache Portable Runtime (APR). This library exposes an abstraction layer over common OS functions and allows writing OS independent code. By using this, if I required something OS specific I would use APR rather than code 2 or 3 versions of the same.

My initial idea was to use APR to handle files and memory mapped IO, in the end I did not use them because libELF had its own implementation of file handling (but I did use them for loading the database as a memory mapped file). However I did find one point where APR was a life saver: memory management. I implemented all my objects with compatibility for APR memory pools, which gave me instant support for dynamic memory and destructors. By using the pool I could create and destroy memory in bulk (for my 'objects') while keeping the code relatively sane.

As for the binary build, the code was simple, I am not sure if I had to add any special flags for each OS, there is a compatibility file, but it had no OS specific code.

# My participation #

Except where I cited sources (like APR, or the x86 reference) I did all the coding, design and testing. 

# What is broken #
Due to time pressure I never finished the code implementation, there was an oversight from the teacher and the project turned out to be far larger than he anticipated. In the end, he was interested in the architectural design and the approach taken to solve the problem, so he was satisfied.

All operand parsers are missing, unfortunately due to the way x86 works, there is no way to know where an instruction starts unless you parse everything of the prior instructions. Due to this, the current implementation only processes one opcode and then skips to the next section.

Also, due to time, only ELF files can be parsed, I could not find a native library for winPE, so I had to code one and I never got around to doing that.
