# 💻 AuraShell – Custom Unix Shell in C

## 📌 Overview

**AuraShell** is a lightweight custom command-line shell developed using the C programming language. It mimics the behavior of a basic Unix/Linux shell by accepting user commands, processing them, and executing system-level operations.

The project demonstrates core concepts of **Operating Systems**, such as process creation, system calls, command parsing, and inter-process communication.

---

## 🎯 Objectives

* Build a custom shell from scratch using C
* Understand how operating systems execute commands
* Learn process management using system calls
* Implement basic shell functionalities

---

## 🚀 Features

* 🖥️ **Command Execution** (e.g., `ls`, `pwd`, `date`)
* 🔁 **Loop-based Shell Interface**
* ⚙️ **Process Creation using `fork()`**
* ▶️ **Program Execution using `exec()` family**
* ⏳ **Process Synchronization using `wait()`**
* 🧩 **Command Parsing (input tokenization)**
* 🚪 **Built-in Commands** like `exit`
* ❌ **Error Handling for Invalid Commands**

---

## 🛠️ Technologies Used

* **C Programming Language**
* **POSIX System Calls**
* Standard Libraries:

  * `stdio.h`
  * `stdlib.h`
  * `unistd.h`
  * `sys/types.h`
  * `sys/wait.h`
  * `string.h`

---

## 📂 Project Structure

```
AuraShell/
│── main.c          # Main shell implementation
│── parser.c        # Command parsing logic (optional)
│── utils.c         # Helper functions (optional)
│── README.md       # Project documentation
```

---

## ⚙️ How AuraShell Works

1. Displays a custom shell prompt
2. Accepts user input from terminal
3. Parses the input into commands and arguments
4. Creates a child process using `fork()`
5. Executes command using `execvp()`
6. Parent process waits for child using `wait()`
7. Repeats until user exits

---

## ▶️ Compilation & Execution

### 1. Compile the code

```
gcc main.c -o aura_shell
```

### 2. Run the shell

```
./aura_shell
```

---

## 💡 Example Commands

```
AuraShell> ls
AuraShell> pwd
AuraShell> echo Hello World
AuraShell> exit
```

---

## 🔮 Future Enhancements

* 🔗 Support for **pipes (`|`)**
* 📂 Input/Output **redirection (`>`, `<`)**
* 🧠 Command history feature
* ⏱️ Background process execution (`&`)
* 🎨 Colored terminal output

---

## ⚠️ Requirements

* GCC Compiler
* Linux / macOS terminal (POSIX supported system)
* Basic knowledge of C programming

---

## 📝 Notes

* This shell supports only basic commands (no advanced scripting yet)
* Commands must follow standard Unix syntax
* Some features may vary depending on the OS environment
* Designed mainly for educational purposes

---

## 📜 License

This project is licensed under the **MIT License**.

You are free to:

* Use
* Modify
* Distribute

With proper attribution.
