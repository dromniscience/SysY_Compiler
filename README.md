# SysY Compiler
### Course Project of *Compiler: Principles & Practice, 2021, Spring*

This repo authored by Ding Rui, dromniscience@gmail.com

Documentation of this lab can be found here: [PKU online doc for SysY compiler lab](https://pku-minic.github.io/online-doc/#/)

### Current Status
- All tests passed. Performance: 292s
- Naive local optimization on Eeyore- & Tigger-level
- Naive register allocation scheme, i.e. heavy loads & stores.

### Usage

If you want to generate an instance of the compiler, simply type
```
cd SysY_Compiler 
make
```
and you shall see an executable file named *compiler* right in this folder.

To clean up all relocatable files, type
```
make clean
```

To clean up all files including the existing compiler, type
```
make cleanall
```

Given a file written in SysY,
```
./compiler -e in.sy -o out.ee     # generate eeyore file
./compiler -t in.sy -o out.tg     # generate tigger file
./compiler -S in.sy -o out.S      # generate RISC-V file
./compiler -h  # helper info
```
 Syntax errors and semantic errors will be reported in a way much similar to clang.

### General Logic of This Compiler

#### SysY -> Eeyore

The most important engineering designs and implementations are captured here (slides that are made on my own):

[My slides](./eeyore.pdf)

All relevant codes in this phase are in the folder named *front*. Contact [README.md](./front/README.md) in this folder for more information.

In fact, I customized AST for each IR, including Eeyore and Tigger. The task is not that formidable at the first glance since a linear data structure would suffice for these two, allowing later opts to kick in.

It is stupid that Eeyore prohibits the use of logic ops such as '<' in an assignment. By virtue of AST for eeyore, we could have it stored as an arithmetic op. Simply translate it to the correct grammar when dumping this AST to the output.

Local optimizations are performed on this level. Among them the most effective one is the removal and the absorption of redundant clauses, which are usually derivatives of excessive declarations of intermediates.

#### Eeyore -> Tigger

All relevant codes in this phase are in the folder named *back*. A simple translation from eeyore to tigger is required. The main task here boils down to storage allocation. Besides, typical optimization on this level is the elimination of redundant loads & stores.

#### Tigger -> RISC-V

All relevant codes in this phase are in *back/tgrep\**. This phase is made up of a simple translation as well. However, there is still room for opts on this level that I have not been seriously considered.

#### What is special about this SysY compiler?
- A user-friendly error reporting interface, whose visual effect mimics Clang. (Though there is still quite a gap since I did not equip my compiler with so many miscellaneous errors and warnings!)
- Fine modularity. I have tried the best to decouple the whole project. The only interaction takes place at the short-circuit code, where flow labels are planted into each AST node.
- High efficiency. Though it is not optimized, for I usually trade-off simplicity for efficiency due to my tight schedule, I have made it quite efficient in some major aspects, which includes exploitations of good C++11/17 features.
- No memory leak. Tested on Valgrind.

### Future Works
- Local Register Assignment
- Opts across basic blocks
- Inter-procedural opts
- Finer check of control flow to report potential discrepancy of return value and the signature of a function at compiling time. 
  *[This is not doable per se, but we are happy even with a compromised solution of this problem.]*
- Efficient way to implement '{}' semantics in local array initialization on RISC-V level.

### Anouncement

The code and other related materials released here are for better communication and knowledge sharing. Please **DO NOT** plagiarize the code or any other materials, or even a part of them.


