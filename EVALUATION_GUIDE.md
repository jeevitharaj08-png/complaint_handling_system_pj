# Complaint Handling System - Technical Evaluation & Viva Defense Guide

Use this technical guide to understand the code architecture, data structures, algorithms, and design decisions so you can answer questions during your project defense.

---

## 1. Modular File Breakdown & Responsibilities

| Header File (`include/`) | Implementation (`src/`) | Responsibility & Purpose |
| :--- | :--- | :--- |
| [`common.h`](file:///c:/Users/Likhith/OneDrive/Desktop/complaint_system/include/common.h) | *(Header only)* | Central definitions: string/buffer length constants, enumerations (`ComplaintStatus`, `ComplaintPriority`, `ComplaintCategory`, `SystemResult`), nested structures (`DateTime`, `ResolutionInfo`), and storage constants. |
| [`utils.h`](file:///c:/Users/Likhith/OneDrive/Desktop/complaint_system/include/utils.h) | [`utils.c`](file:///c:/Users/Likhith/OneDrive/Desktop/complaint_system/src/utils.c) | Defensive I/O library: safe line reading with `fgets` and stdin flushing, numeric input range validation, calendar date/time capture (`utils_get_current_time`), timestamp hour difference calculation (`utils_hours_difference`), and string-enum converters. |
| [`customer.h`](file:///c:/Users/Likhith/OneDrive/Desktop/complaint_system/include/customer.h) | [`customer.c`](file:///c:/Users/Likhith/OneDrive/Desktop/complaint_system/src/customer.c) | Customer entity management: dynamic array store with capacity doubling, registration with sequential ID (`CST-XXXX`), duplicate phone & email checks, and fast lookups by ID/phone/email. |
| [`agent.h`](file:///c:/Users/Likhith/OneDrive/Desktop/complaint_system/include/agent.h) | [`agent.c`](file:///c:/Users/Likhith/OneDrive/Desktop/complaint_system/src/agent.c) | Support staff directory: manages agents across college departments (Academic, Hostel, Infrastructure, Finance, Examination), maintains live active ticket workload counters, and implements the **least-loaded agent assignment algorithm**. |
| [`complaint.h`](file:///c:/Users/Likhith/OneDrive/Desktop/complaint_system/include/complaint.h) | [`complaint.c`](file:///c:/Users/Likhith/OneDrive/Desktop/complaint_system/src/complaint.c) | Core business engine: manages complaints in an in-memory **Doubly Linked List** (`ComplaintList` / `ComplaintNode`), enforces lifecycle state transitions, handles manual & auto-escalation, records resolution, and captures satisfaction ratings. |
| [`audit.h`](file:///c:/Users/Likhith/OneDrive/Desktop/complaint_system/include/audit.h) | [`audit.c`](file:///c:/Users/Likhith/OneDrive/Desktop/complaint_system/src/audit.c) | Audit trail engine: builds per-complaint in-memory **Singly Linked Lists** (`AuditNode`) for chronological timeline inspection and maintains an append-only persistent text journal in `data/audit.log`. |
| [`storage.h`](file:///c:/Users/Likhith/OneDrive/Desktop/complaint_system/include/storage.h) | [`storage.c`](file:///c:/Users/Likhith/OneDrive/Desktop/complaint_system/src/storage.c) | Binary file persistence: structured serialization with magic headers (`0x434D5053`, `0x43555354`, `0x41474E54`) and schema version checks; uses atomic write-and-rename to prevent file corruption. |
| [`search.h`](file:///c:/Users/Likhith/OneDrive/Desktop/complaint_system/include/search.h) | [`search.c`](file:///c:/Users/Likhith/OneDrive/Desktop/complaint_system/src/search.c) | Multi-criteria query engine: linear search traversals over the doubly linked list filtering by ID, customer ID, status, priority, agent ID, or description keyword. |
| [`report.h`](file:///c:/Users/Likhith/OneDrive/Desktop/complaint_system/include/report.h) | [`report.c`](file:///c:/Users/Likhith/OneDrive/Desktop/complaint_system/src/report.c) | Statistical reporting: aggregate distributions by status, priority, and category with visual ASCII bars; calculates resolution rates, agent workloads, and average satisfaction ratings. |
| [`ui.h`](file:///c:/Users/Likhith/OneDrive/Desktop/complaint_system/include/ui.h) | [`ui.c`](file:///c:/Users/Likhith/OneDrive/Desktop/complaint_system/src/ui.c) | Interactive console user interface: orchestrates 17 menu workflows with error handling, sub-menus, formatted tables, and user prompts. |
| *(None)* | [`main.c`](file:///c:/Users/Likhith/OneDrive/Desktop/complaint_system/src/main.c) | System entry point: initializes directories and memory stores, loads persistent binary records, seeds sample data if fresh, runs the UI loop, and cleanly frees all heap memory on shutdown. |

---

## 2. Core Data Structures & Why They Were Chosen

### 1. `DateTime` (Nested Structure)
```c
typedef struct {
    int year, month, day, hour, minute, second;
} DateTime;
```
- **Why chosen:** Standardizes date and time across the entire system without relying on platform-specific string formats. Embedded directly inside `Complaint`, `Customer`, `ResolutionInfo`, and `AuditNode`.

### 2. `ResolutionInfo` (Nested Structure)
```c
typedef struct {
    char resolved_by_agent_id[MAX_ID_LEN];
    DateTime resolved_at;
    char resolution_notes[MAX_NOTES_LEN];
    int satisfaction_rating; /* 1 - 5 */
} ResolutionInfo;
```
- **Why chosen:** Encapsulates resolution metadata as a logical unit inside `Complaint`. Keeps resolution details separate from ticket intake fields.

### 3. `AuditNode` (Singly Linked List Node)
```c
typedef struct AuditNode {
    DateTime timestamp;
    char complaint_id[MAX_ID_LEN];
    char action[MAX_ACTION_LEN];
    char actor[MAX_NAME_LEN];
    char remarks[MAX_NOTES_LEN];
    struct AuditNode *next;
} AuditNode;
```
- **Why chosen:** Each ticket has an unpredictable number of lifecycle actions (e.g., created, assigned, updated, escalated, resolved, closed). A singly linked list allows dynamic growth with zero wasted memory.

### 4. `Complaint` and `ComplaintNode` (Doubly Linked List Node)
```c
typedef struct {
    char complaint_id[MAX_ID_LEN];
    char customer_id[MAX_ID_LEN];
    ComplaintCategory category;
    char description[MAX_DESC_LEN];
    ComplaintPriority priority;
    ComplaintStatus status;
    char assigned_agent_id[MAX_ID_LEN];
    DateTime created_at;
    DateTime updated_at;
    ResolutionInfo resolution;
    AuditNode *audit_history_head;
} Complaint;

typedef struct ComplaintNode {
    Complaint data;
    struct ComplaintNode *prev;
    struct ComplaintNode *next;
} ComplaintNode;
```
- **Why chosen:** A **Doubly Linked List** allows $O(1)$ insertion at tail, bidirectional traversal for reports/sorting, and efficient deletion of any node without traversing from the head.

### 5. `Customer` and `SupportAgent` (Contiguous Dynamic Arrays)
- **Why chosen:** Customers and agents are frequently looked up by index or ID. Dynamic arrays provide $O(1)$ random access, cache-friendly memory locality, and easy binary serialization.

### 6. `StorageFileHeader` and `ComplaintDiskRecord` (Persistence Structures)
```c
typedef struct {
    uint32_t magic;
    uint32_t version;
    uint32_t record_count;
    uint32_t next_id_seq;
} StorageFileHeader;
```
- **Why chosen:** Disk records cannot safely store memory pointers (`prev`, `next`, `audit_history_head`). `ComplaintDiskRecord` defines a fixed-size, disk-serializable representation, and `StorageFileHeader` verifies file integrity via magic numbers before reading records.

---

## 3. Key Algorithms & Implementation Details

### 1. Least-Loaded Agent Assignment Algorithm
- **Location:** [`src/agent.c` - `agent_find_least_loaded()`](file:///c:/Users/Likhith/OneDrive/Desktop/complaint_system/src/agent.c#L99-L138)
- **How it works:**
  1. Scans all active agents to find the minimum active complaint count (`min_tickets`).
  2. If a department is specified (e.g. `HOSTEL`), it searches among agents who hold that `min_tickets` workload for a departmental specialist.
  3. If found, it assigns that specialist. Otherwise, it picks the first active agent holding `min_tickets`.
- **Result:** Workloads stay balanced across the support team while still giving preference to domain experts.

### 2. Auto-Escalation SLA Aging Algorithm
- **Location:** [`src/complaint.c` - `complaint_auto_escalate_critical()`](file:///c:/Users/Likhith/OneDrive/Desktop/complaint_system/src/complaint.c#L274-L315) and [`src/utils.c` - `utils_hours_difference()`](file:///c:/Users/Likhith/OneDrive/Desktop/complaint_system/src/utils.c#L68-L80)
- **How it works:**
  1. Converts ticket `created_at` and current system time into Unix epoch seconds using `mktime()`.
  2. Computes elapsed time in whole hours via `difftime() / 3600.0`.
  3. If ticket is in an active state (`OPEN`, `ASSIGNED`, `IN_PROGRESS`), has `PRIORITY_CRITICAL`, and `elapsed >= threshold_hours` (default 24h), it auto-escalates to `STATUS_ESCALATED` and logs an audit entry.

### 3. Atomic Binary Persistence
- **Location:** [`src/storage.c` - `storage_save_complaints()`](file:///c:/Users/Likhith/OneDrive/Desktop/complaint_system/src/storage.c#L28-L71)
- **How it works:**
  1. Opens a temporary file `data/complaints.tmp`.
  2. Writes the 16-byte `StorageFileHeader` with magic number `0x434D5053`.
  3. Writes each complaint's `ComplaintDiskRecord` sequentially.
  4. Flushes (`fflush`) and closes the file.
  5. Atomically renames `complaints.tmp` to `complaints.dat`.
- **Result:** If power is lost or the application crashes mid-write, the existing database file is never corrupted.

---

## 4. Likely Viva / Examiner Questions & Answers

**Q1: Why did you use a Doubly Linked List for complaints instead of a static array?**
> *"A static array has a fixed compile-time capacity and wastes memory for unallocated slots, or risks buffer overflow if the college receives more complaints than expected. A doubly linked list allows dynamic heap allocation as tickets arrive, provides $O(1)$ insertion at tail, and allows bidirectional traversal."*

**Q2: How do you prevent memory leaks when the program terminates?**
> *"Every allocation via `malloc` or `realloc` is tracked. The function `complaint_list_cleanup()` traverses the doubly linked list, frees every nested `AuditNode` singly linked list for each ticket, frees the complaint nodes, and resets pointers to `NULL`. Similarly, `customer_store_cleanup()` and `agent_store_cleanup()` free the dynamic arrays. In `main()`, these cleanup routines are executed before `return 0`."*

**Q3: How do you store structs with pointers into a binary file?**
> *"In-memory pointers (`prev`, `next`, `audit_history_head`) point to ephemeral RAM addresses that become invalid once the process exits. We use a dedicated flat structure `ComplaintDiskRecord` that stores pure data. On saving, in-memory complaints are converted to disk records. On loading, nodes are dynamically allocated and relinked in memory."*

**Q4: How does your system ensure data integrity against file corruption?**
> *"Every binary file starts with a `StorageFileHeader` containing a 4-byte magic number (`MAGIC_COMPLAINTS = 0x434D5053`) and a schema version. When reading, if the magic number or version does not match, or if the file is truncated, the loader rejects the corrupt file with an error code rather than crashing or reading garbage data."*

**Q5: How do you handle user input safely without buffer overflow?**
> *"We do not use `gets()` or unbounded `scanf("%s")`. All inputs go through `utils_read_line()`, which wraps `fgets(buffer, max_len, stdin)`. It strips newline characters, trims leading/trailing whitespace, and flushes any leftover characters in `stdin` if the user entered more text than the buffer capacity."*
