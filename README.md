# Git Assume-Unchanged Manager

In Git `assume-unchanged` is a command that allows you to mark files as "ignored" by Git for local changes, without removing them from the repository. This is particularly useful for:
- Ignoring configuration files or local environment files that should not interfere with repository updates.
- Any files that that may have changes that you don't want to be committed, but are needed in the repo.

This tool provides a simple CLI interface to help manage the `assume-unchanged` state for files in your Git repository.

### Dependencies and Target System
This program is designed for **Linux systems**, specifically tested on Ubuntu 22.04, where dependencies are included by default. On older systems, you may need to install:
- `g++` (GNU C++ compiler)
- `libncurses-dev` (ncurses development library)

Refer to the [Dependencies Installation](#dependencies-installation) section below for instructions if required.

---

## Run / Install

### Run

1. Clone the repository or copy the source code to your local machine:
   ```bash
   git clone https://github.com/u2084511felix/gitassume
   cd gitassume 
   ```

2. 
   a) Create a symbolic link to make the binary executable from anywhere (e.g., `/usr/local/bin`):
   ```bash
   sudo ln -s $(pwd)/gitassume /usr/local/bin/gitassume
   ```

   or

   b) Compile the program yourself:
   ```bash
   g++ -o gitassume assume.cpp -lncurses
   ```
   Then do step 2(a).

3. Run the program from anywhere:
   ```bash
   gitassume
   ```

---

## Keyboard Controls

| Key         | Function                                        |
|-------------|------------------------------------------------|
| **UP/DOWN** | Navigate through the file list                 |
| **ENTER**   | Toggle the `assume-unchanged` state of a file  |
| **TAB**     | Switch between "Available to Assume" and "Currently Assumed" |
| **Q**       | Quit the application                           |

---

## Notes

- Ensure the directory you run the program in is a Git repository.
- Files already tracked by Git are displayed for toggling.

---

## Dependencies Installation
If running on older systems (e.g., Ubuntu versions prior to 22.04), install the required dependencies:

```bash
sudo apt update
sudo apt install g++ libncurses-dev
```
