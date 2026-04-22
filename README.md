# Course Registration System

A multi-user **course registration system** built in C using a TCP client-server architecture. Multiple clients can connect simultaneously and interact through a terminal interface. Supports three roles — Admin, Professor, and Student — each with their own set of operations.

## Features

**Admin**
- Add students and professors (auto-generates login credentials)
- View all student and professor records
- Activate or deactivate student accounts
- Modify student and professor details
- Reset any student's or professor's password

**Professor**
- Add, modify, and remove courses from the catalog
- View enrolled students per course
- Seat management with automatic unenrollment when seats are reduced

**Student**
- Browse all available courses
- Enroll in and drop courses
- View current enrollments
- Change own password (requires current password verification)

## Visit the Deployed Link
```
```

## Prerequisites

### macOS
Install Xcode Command Line Tools (includes `gcc`, `make`, `clang`):
```bash
xcode-select --install
```

### Linux (Debian / Ubuntu)
```bash
sudo apt update
sudo apt install build-essential
```

### Linux (Fedora / RHEL / CentOS)
```bash
sudo dnf install gcc make
```

### Linux (Arch)
```bash
sudo pacman -S base-devel
```

### Windows
The server uses POSIX APIs (`fork`, `fcntl`, POSIX file locking) that are **not available natively on Windows**. Use one of the following:

**Option A — WSL2 (recommended)**
1. Install WSL2: open PowerShell as Administrator and run:
   ```powershell
   wsl --install
   ```
2. Restart your PC, then open the Ubuntu terminal.
3. Inside WSL2, install build tools:
   ```bash
   sudo apt update && sudo apt install build-essential
   ```
4. Clone or copy the project into your WSL2 home directory and follow the Linux instructions below.

**Option B — MSYS2**
1. Download and install MSYS2 from [msys2.org](https://www.msys2.org).
2. Open the **MSYS2 MinGW 64-bit** terminal and run:
   ```bash
   pacman -S mingw-w64-x86_64-gcc make
   ```
3. Note: `fork()` is not available in MinGW. The server will not compile as-is. WSL2 is strongly preferred.

## Build

Clone the repository and build from the project root:

```bash
git clone <repo-url>
cd SS-Mini-Project
make
```

This produces two executables: `server` and `client`.

## Initialise Data Files

Run this **once** before starting the server for the first time, or any time you want to wipe all data and start fresh:

```bash
make init
```

> **Warning:** `make init` deletes all existing students, professors, courses, and enrollments. Do not run it on a system with data you want to keep.

This creates the `files/` directory contents:

| File | Purpose |
|---|---|
| `files/adminCred` | Admin credentials |
| `files/accountCnt` | Global ID counters |
| `files/studentDetails` | Student records |
| `files/teacherDetails` | Professor records |
| `files/courses` | Course catalog |
| `files/enrollments` | Enrollment records |

## Run

Open **two terminals** in the project directory.

**Terminal 1 — start the server:**
```bash
./server
```
The server listens on **port 9008**.

**Terminal 2 — connect a client:**
```bash
./client                   # connects to localhost (default)
./client 203.0.113.42      # connects to a remote server by IP
```

You can open as many client terminals as you need. Each connection is handled in a separate forked process.

## Default Credentials

After `make init`:

| Role | Username | Password |
|---|---|---|
| Admin | `root` | `root` |
| Student | `MT<id>` e.g. `MT1` | `iiitb` |
| Professor | `PROF<id>` e.g. `PROF1` | `iiitbp` |

Student and professor credentials are printed to the admin's terminal when accounts are created.

## Typical First-Time Workflow

1. `make && make init`
2. Start the server: `./server`
3. Connect a client: `./client`
4. Login as **Admin** (root / root)
5. Add a professor → note the generated `PROF<id>`
6. Add students → note the generated `MT<id>`s
7. Open another client, login as the professor, add courses
8. Open another client, login as a student, enroll in courses

## Project Structure

```
SS-Mini-Project/
├── server.c            # Server entry point (accept, fork, dispatch)
├── client.c            # Client terminal UI
├── structures.h        # Shared data structures (Student, Professor, Course, ...)
├── Makefile
├── files/              # Binary data files (created by make init)
└── functions/
    ├── menu.h          # Menu string constants
    ├── login.h         # Login router
    ├── admin.h         # Admin operations
    ├── professor.h     # Professor operations
    ├── student.h       # Student operations
    ├── course.h        # Course lookup and seat helpers
    ├── enrollment.h    # Enrollment record lookup
    ├── getSet.h        # Global ID counter read/write
    └── setAdmin.c      # Init utility (compiled by make init)
```

## Clean

Remove compiled binaries:
```bash
make clean
```

## Technical Notes

- **Concurrency:** each client connection is served by a forked child process. File access is protected with `fcntl` read/write locks to prevent race conditions.
- **Data storage:** all records are stored as fixed-size binary structs in flat files. Records are addressed by offset (`(id-1) * sizeof(struct)`).
- **Port:** hardcoded to `9008`. To change it, edit `server.c` and `client.c`.
- **Platform:** requires a POSIX-compatible system (Linux, macOS, or WSL2 on Windows).
