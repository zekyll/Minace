Minace v.1.0
============

Minace (aka Minace Is Not A Chess Engine) is a UCI chess engine for Linux and Windows, written in C++11. Current ELO rating has not been accurately measured but based on the engines that I have tested it against I would estimate it to be somewhere around 1800 to 2000.

Building
--------
The repository contains project files for NetBeans 7.4. Building from command line is also possible:

```
make CONF=Release
```

The executable is generated in ```./Minace/dist/Release/GNU-Linux-x86/minace``` on Linux. Building for either 32-bit or 64-bit systems should work.

The build has only been tested with g++ 4.8 on Linux and MinGW 4.8 on Windows. It relies heavily on C++11 features so it probably will not work on older compilers.

Usage
-----
Add the executable path in any UCI compliant chess GUI (e.g. XBoard, Arena, PyChess), or run the executable from command line if you want to manually interact with it (see protocol specs at http://wbec-ridderkerk.nl/html/UCIProtocol.html).

Notes about UCI support
-----------------------
 - The only supported UCI option is "Hash" for setting hash size.
 - Pondering is not supported.
 - Mate search and restricted search are not supported
 - Provided info output is very limited and for example PV may not be correct

Version history
---------------

**V.1.0 (2014-01-11)**

 - First release
 - Engine features: chess rules working 99.9% correctly, alpha beta pruning, principal variation search, transposition table, Zobrist hashing, quiescence search, null move reductions, move generation using bitboards, magic bitboards for sliding piece moves, move ordering, killer heuristic
 - UCI Features: All the basic commands, supports "Hash" option for settings hash size

