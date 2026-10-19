# Complaint Handling System - Requirements Traceability & Verification Audit

This document maps each specification requirement of the College Complaint Handling System to its implementing source file(s), header interface(s), and automated verification test functions.

---

| Req # | Requirement Description | Implementation Module(s) | Header File(s) | Verification Test in `tests/test_runner.c` | Verification Status |
| :---: | :--- | :--- | :--- | :--- | :---: |
| **0** | **Project Architecture & Build Tooling**<br>Modular `.c`/`.h` files, C11 standard, cross-platform `Makefile`, Windows `build.bat`, VS Code `.vscode/tasks.json` & `launch.json`. | `src/utils.c`, `src/main.c` | `include/common.h`, `include/utils.h` | Verified via clean compilation under GCC `-Wall -Wextra -Wpedantic -std=c11`. | 🟢 **Verified** |
| **1** | **Customer Management**<br>Unique Customer ID (`CST-XXXX`), full name, phone, email, address, registration timestamp, duplicate checks for phone and email, lookups by ID/phone/email. | `src/customer.c` | `include/customer.h` | `test_customer_management()`: validates ID sequencing, empty inputs, duplicate phone & email rejections, and lookups. | 🟢 **Verified** (13 assertions) |
| **2** | **Complaint Registration**<br>Unique Complaint ID (`CMP-XXXX`), customer linkage, category, description, priority, auto-populated timestamp, doubly linked list storage. | `src/complaint.c` | `include/complaint.h` | `test_complaint_lifecycle_and_state_machine()`: validates node allocation, sequence generator, NULL/empty parameter guards. | 🟢 **Verified** (10 assertions) |
| **3** | **Complaint Assignment & Support Agents**<br>Support agent directory, manual assignment, automated least-loaded assignment (strictly fewest active complaints with specialist tie-breaking), live workload counters. | `src/agent.c`, `src/complaint.c` | `include/agent.h`, `include/complaint.h` | `test_agent_and_assignment()`: validates default seeding, least-loaded selection, tie-breaking, workload increment/decrement. | 🟢 **Verified** (11 assertions) |
| **4** | **Status Tracking & State Machine**<br>`OPEN`, `ASSIGNED`, `IN_PROGRESS`, `ESCALATED`, `RESOLVED`, `CLOSED`. Complete enforcement of valid and invalid transitions. | `src/complaint.c` | `include/complaint.h` | `test_complaint_lifecycle_and_state_machine()`: tests valid lifecycle progression and 12 distinct invalid transition rejections. | 🟢 **Verified** (19 assertions) |
| **5** | **Priority Management**<br>`LOW`, `MEDIUM`, `HIGH`, `CRITICAL` with mandatory justification reason logged in audit trail; rejection on closed tickets. | `src/complaint.c` | `include/complaint.h` | `test_complaint_lifecycle_and_state_machine()`: validates priority change, audit log entry creation, and rejection on closed tickets. | 🟢 **Verified** (4 assertions) |
| **6** | **Escalation Rules & Management**<br>Manual escalation + automatic SLA escalation for `CRITICAL` complaints exceeding configurable threshold (default 24h, tested with timestamp math). | `src/complaint.c`, `src/utils.c` | `include/complaint.h`, `include/utils.h` | `test_auto_escalation_rules()`: tests manual escalation, 48h aging simulation on critical tickets, non-escalation of non-critical tickets. | 🟢 **Verified** (7 assertions) |
| **7** | **Resolution Details & Closure**<br>Resolution notes, resolver agent ID, resolved timestamp, customer satisfaction rating (1-5 stars) upon closure. | `src/complaint.c` | `include/complaint.h` | `test_complaint_lifecycle_and_state_machine()`: tests resolver notes, agent workload decrement, rating range validation (<1 and >5 rejected). | 🟢 **Verified** (8 assertions) |
| **8** | **Multi-Criteria Search Engine**<br>Search by Complaint ID, Customer ID, Status, Priority, Assigned Agent, or Keyword substring in description. | `src/search.c` | `include/search.h` | `test_search_and_query_engine()`: tests all 6 query criteria and verifies non-existent query handling. | 🟢 **Verified** (7 assertions) |
| **9** | **Complaint History & Audit Logs**<br>In-memory singly linked list (`AuditNode`) per complaint + append-only disk journal (`data/audit.log`) with formatted timestamps and actions. | `src/audit.c` | `include/audit.h` | `test_complaint_lifecycle_and_state_machine()`: tests in-memory event chain ($\ge 6$ events per ticket) and persistent disk writes. | 🟢 **Verified** (2 assertions) |
| **10**| **Summary Reports & Executive Analytics**<br>Breakdowns and distribution bars by status, priority, category, agent performance metrics, executive KPI dashboard. | `src/report.c` | `include/report.h` | `test_summary_reports_and_metrics()`: tests mathematical parity between ticket counts, percentage distributions, and dashboard KPI calculations. | 🟢 **Verified** (2 assertions) |
| **--**| **Persistent Binary Storage**<br>Fixed-size binary records with magic number (`0x434D5053`, `0x43555354`, `0x41474E54`) and schema version validation, atomic write/flush. | `src/storage.c` | `include/storage.h` | `test_file_persistence_and_corruption()`: tests save/reload parity, corrupted magic header rejection, and truncated file rejection. | 🟢 **Verified** (11 assertions) |
| **--**| **Interactive Console User Interface**<br>Menu-driven CLI with safe input validation, auto-reprompting on invalid values, formatted ANSI card/table views. | `src/ui.c`, `src/main.c` | `include/ui.h` | Validated through interactive testing and automated pipeline tests. | 🟢 **Verified** |

---

### Verification Summary
- **Total Automated Assertions:** 94
- **Passed:** 94
- **Failed:** 0
- **Compiler:** GCC 15.2.0 (C11, `-Wall -Wextra -Wpedantic`)
