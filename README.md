# Extended System Programming Laboratory Mini‑Projects (7 Labs)

Seven incremental coding labs completed for the *Extended System Programming Laboratory* course at Ben‑Gurion University.  Each lab is its own self‑contained mini‑project; together they chart my journey from basic C programming through low‑level ELF hacking and shell implementation with job control.

---

## Table of Contents

1. [Lab A – Intro to C & Encoder](#lab-a--intro-to-c--encoder)
2. [Lab 1 – Memory, Pointers & OO‑style C](#lab-1--memory-pointers--oo-style-c)
3. [Lab 2 – Mini‑Shell (processes, exec, signals)](#lab-2--mini-shell-processes-exec-signals)
4. [Lab B – Debugging, Linked Lists & Virus Detector](#lab-b--debugging-linked-lists--virus-detector)
5. [Lab 3 – Assembly & Direct System Calls](#lab-3--assembly--direct-system-calls)
6. [Lab 4 – ELF Internals & HexeditPlus](#lab-4--elf-internals--hexeditplus)
7. [Lab C – Advanced Shell: Pipes, Jobs & History](#lab-c--advanced-shell-pipes-jobs--history)
8. [Global Setup](#global-setup)
9. [License](#license)

---

## Lab A – Intro to C & Encoder

**Folder:** `labA/`

|                   | Details                                                                                                                    |
| ----------------- | -------------------------------------------------------------------------------------------------------------------------- |
| **One‑liner**     | Command‑line additive/subtractive text‑encoder with optional I/O redirection.                                              |
| **Core tech**     | `C99`, Makefile build, standard streams (`fgetc`/`fputc`).                                                                 |
| **Key learnings** | Makefile maintenance • CLI parsing (`argc/argv`) • ASCII & cyclic key encoding • Debug‑mode pattern writing to **stderr**. |

```bash
cd labA
make        # produces `encoder`
./encoder +E1234 -i plain.txt -o cipher.txt  # example
```

---

## Lab 1 – Memory, Pointers & OO‑style C

**Folder:** `lab1/`

|                   | Details                                                                                                                                                               |
| ----------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **One‑liner**     | A set of micro‑programs exploring virtual memory layout, pointer arithmetic and function‑pointer‑driven “map”.                                                        |
| **Core tech**     | `gdb`, `sizeof`, function pointers, dynamic allocation.                                                                                                               |
| **Key learnings** | Debugging seg‑faults • Stack/heap/code dissection • Array/pointer relationships • Manual polymorphism via function pointers • Memory discipline with `malloc`/`free`. |

---

## Lab 2 – Mini‑Shell (processes, exec, signals)

**Folder:** `lab2/`

|                   | Details                                                                                                                                              |
| ----------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------- |
| **One‑liner**     | Lightweight interactive shell supporting foreground/background execution, I/O redirection and basic signal handling.                                 |
| **Core tech**     | `fork`, `execvp`, `waitpid`, POSIX signals, `LineParser` helper.                                                                                     |
| **Key learnings** | Process lifecycle control • Custom prompt via `getcwd` • Signal handlers for `SIGINT`, `SIGTSTP`, `SIGCONT` • Debug flag (`-d`) for verbose tracing. |

---

## Lab B – Debugging, Linked Lists & Virus Detector

**Folder:** `labB/`

|                   | Details                                                                                                      |
| ----------------- | ------------------------------------------------------------------------------------------------------------ |
| **One‑liner**     | Memory‑safe virus‑signature scanner built on a custom linked‑list loader with hex‑dump utility.              |
| **Core tech**     | `Valgrind`, linked lists, binary‑file I/O, naïve pattern matching.                                           |
| **Key learnings** | Leak/illegal‑access hunting • Dynamic data structures • Reading/writing binary safely • Hex dump formatting. |

---

## Lab 3 – Assembly & Direct System Calls

**Folder:** `lab3/`

|                   | Details                                                                                                |
| ----------------- | ------------------------------------------------------------------------------------------------------ |
| **One‑liner**     | Re‑implements the encoder entirely in 32‑bit NASM using raw Linux INT 80h syscalls.                    |
| **Core tech**     | NASM, ELF linking (`start.s`), custom `system_call` wrapper, `ld`.                                     |
| **Key learnings** | Mixing C & ASM • Manual `_start` entry • Syscall ABI • Makefile pipelines for mixed‑language projects. |

---

## Lab 4 – ELF Internals & HexeditPlus

**Folder:** `lab4/`

|                   | Details                                                                                                             |
| ----------------- | ------------------------------------------------------------------------------------------------------------------- |
| **One‑liner**     | Interactive menu‑driven hex‑editor for 32‑bit ELF binaries, featuring file→memory copy and unit‑size aware display. |
| **Core tech**     | `readelf`, custom CLI menus, offset arithmetic, formatted I/O helpers.                                              |
| **Key learnings** | ELF header navigation • Binary patching offsets • Toggleable debug/display modes • Safe buffer handling.            |

---

## Lab C – Advanced Shell: Pipes, Jobs & History

**Folder:** `labC/`

|                   | Details                                                                                                              |
| ----------------- | -------------------------------------------------------------------------------------------------------------------- |
| **One‑liner**     | Extends the mini‑shell with single‑pipe support and in‑shell job‑control commands (`procs`, `stop`, `wake`, `term`). |
| **Core tech**     | `dup`, `pipe`, custom process table (linked list), `SIGSTOP/SIGCONT`, history concept.                               |
| **Key learnings** | Inter‑process I/O redirection • Descriptor hygiene • Job‑control signals • Shell extensibility patterns.             |

---

## Global Setup

All labs target **32‑bit Linux** (tested on Ubuntu 20.04) and rely solely on system libraries or plain NASM.  Each folder contains its own `Makefile` and README snippet—`cd` into a lab directory and run `make` to build.

> **Tip:** Some projects (lab3, lab4) assume `gcc -m32`, `ld -m elf_i386`, and `nasm` are installed. On Ubuntu/Debian:
>
> ```bash
> sudo apt install gcc-multilib nasm build-essential
> ```

---

## License

Unless noted otherwise in a specific folder, all source code in this repository is released under the **MIT License**.  See [`LICENSE`](LICENSE) for details.

---

### Acknowledgements

Assignments provided by the OS teaching staff at BGU.  Huge thanks to my lab partners and TAs for feedback along the way.
