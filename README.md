# College Complaint Handling System (C11)

A modular, defensive C11 Complaint Handling System developed for an academic/college project. Engineered with strict separation of concerns, MISRA C-informed safety rules, structured binary persistence, and human-readable audit journals.

---

## Key Features

1. **Customer Management**: Register students/customers with ID (`CST-XXXX`), name, phone, email, and address. Prevents duplicate phone registrations.
2. **Complaint Registration**: Auto-generated ticket ID (`CMP-XXXX`), customer linkage, category, detailed description, priority, and timestamp.
3. **Dual Agent Assignment**:
   - **Automated Assignment**: Dispatches tickets to the least-loaded available agent (fewest active complaints). Prioritizes category specialists.
   - **Manual Assignment**: Directly select any active staff member from the agent directory.
4. **Enforced Status State Machine**:
   - States: `OPEN` $\to$ `ASSIGNED` $\to$ `IN_PROGRESS` $\to$ `ESCALATED` $\to$ `RESOLVED` $\to$ `CLOSED`.
   - Business rule enforcement: prevents closing unresolved complaints without resolution notes.
5. **Priority Management**: `LOW`, `MEDIUM`, `HIGH`, `CRITICAL` with logged justifications.
6. **Configurable Escalation Rules**:
   - **Manual Escalation**: Elevates tickets to `CRITICAL` and marks them as `ESCALATED` with recorded reason.
   - **Automated SLA Escalation**: Calculates timestamp elapsed age; automatically escalates `CRITICAL` tickets open longer than a configurable threshold (default: 24 hours).
7. **Resolution & Customer Feedback**: Tracks resolution timestamp, resolver agent ID, detailed notes, and captures customer satisfaction rating (1 to 5 stars) upon closure.
8. **Multi-Criteria Search Engine**: Fast queries by Complaint ID, Customer ID, Status, Priority, Agent ID, or substring keyword match in description.
9. **Dual Audit Logging**:
   - In-memory singly linked list (`AuditNode`) recording every lifecycle event per ticket.
   - Persistent append-only text journal in `data/audit.log`.
10. **Executive Analytics & Reporting**: Summary breakdowns by status, priority, category, agent performance metrics, and high-level KPI dashboard.
11. **Persistent Binary Storage**: Fixed-size binary records with magic number header validation and atomic writes (`data/complaints.dat`, `data/customers.dat`, `data/agents.dat`).

---

## Project Structure

```
complaint_system/
├── Makefile                    # Cross-platform build script
├── build.bat                   # 1-click Windows compilation script
├── README.md                   # Project documentation
├── .vscode/
│   ├── tasks.json              # VS Code build and test tasks
│   └── launch.json             # VS Code GDB debugger configuration
├── include/                    # Header interfaces (.h)
│   ├── common.h                # Types, enums, limits, and nested structs
│   ├── utils.h                 # Safe input, string cleaners, and datetime math
│   ├── customer.h              # Customer entity and store prototypes
│   ├── agent.h                 # Support agent and workload prototypes
│   ├── complaint.h             # Core complaint doubly linked list prototypes
│   ├── audit.h                 # Audit trail logging prototypes
│   ├── storage.h               # Binary serialization prototypes
│   ├── search.h                # Search engine query prototypes
│   ├── report.h                # Statistical reporting prototypes
│   └── ui.h                    # Interactive console UI prototypes
├── src/                        # Implementations (.c)
│   ├── main.c                  # Lifecycle initialization, storage load, and cleanup
│   ├── utils.c                 # Input sanitization, date formatting, and enum parsers
│   ├── customer.c              # Customer dynamic store and lookups
│   ├── agent.c                 # Agent directory and least-loaded dispatch
│   ├── complaint.c             # Doubly linked list ticket engine
│   ├── audit.c                 # Timeline logging and log file viewer
│   ├── storage.c               # Binary header validation and atomic writes
│   ├── search.c                # Multi-criteria filtering engine
│   ├── report.c                # Graphical ASCII summary reports
│   └── ui.c                    # Console menus and user prompts
├── data/                       # Persistent runtime data
│   ├── complaints.dat          # Binary complaint records
│   ├── customers.dat           # Binary customer records
│   ├── agents.dat              # Binary agent records
│   └── audit.log               # Human-readable append-only log file
└── tests/
    └── test_runner.c           # Automated verification test suite (94 assertions)
```

---

## How to Build and Run

### Option 1: Using `build.bat` on Windows (MinGW GCC)
Double-click `build.bat` or run in terminal:
```powershell
.\build.bat
```
Run the application:
```powershell
.\bin\complaint_system.exe
```
Run the automated test runner:
```powershell
.\bin\test_runner.exe
```

### Option 2: Using `Makefile`
```bash
make all        # Compile main application
make test       # Build and execute test runner
make run        # Run interactive application
make clean      # Clean build artifacts
```

### Option 3: In VS Code
- Press `Ctrl + Shift + B` to trigger default build task.
- Press `F5` to start debugging with GDB.
- Run test task: `Terminal` $\to$ `Run Task...` $\to$ `C/C++: Run Automated Tests`.
