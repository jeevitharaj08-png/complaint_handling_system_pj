# Complaint Handling System - Project Demonstration Script

Follow this step-by-step 5-minute demonstration guide during your project evaluation to showcase all 10 major requirements in an intuitive, flawless sequence.

---

### Step 0: Start the Application
In your terminal, run:
```powershell
.\bin\complaint_system.exe
```
*Examiner Note:* Point out that the system automatically initializes default student records and support staff, and prepares binary persistence.

---

### Step 1: Customer Management
1. Enter `12` to open the **Customer Management Menu**.
2. Enter `4` to **List All Registered Customers**.
   - *Highlight:* Displays pre-seeded students (`CST-1001` - Rahul Sharma, `CST-1002` - Priya Nair).
3. Enter `1` to **Register New Customer**:
   - Full Name: `Ananya Roy`
   - Phone Number: `9876501234`
   - Email Address: `ananya.roy@college.edu`
   - Address: `Hostel Block B, Room 310`
   - *Highlight:* Automatic ID generation (`CST-1003`) and duplicate phone/email detection.
4. Enter `0` to return to the Main Menu.

---

### Step 2: Register a New Complaint
1. Enter `1` to **Register New Complaint**.
2. Enter Customer ID: `CST-1003` (or enter `NEW` if registering on the fly).
3. Select Category: Choose `2` (`HOSTEL`).
4. Select Priority: Choose `3` (`HIGH`).
5. Enter Description: `Ceiling fan making loud grinding noise and sparking occasionally`.
6. Prompt: *"Would you like to auto-assign this ticket to the least-loaded agent now?"*
   - Choose `2` (`No - Keep in OPEN queue`).
   - *Highlight:* Unique Complaint ID `CMP-1001` generated; timestamp auto-captured; ticket starts in `STATUS_OPEN`.

---

### Step 3: Demonstrate Support Agents & Least-Loaded Auto-Assignment
1. Enter `13` $\to$ `2` to **View All Agents & Live Workloads**.
   - *Highlight:* Table showing all 6 departments and their current active ticket counts (all 0).
   - Enter `0` to return to Main Menu.
2. Enter `2` to **Assign / Reassign Complaint**.
3. Enter Complaint ID: `CMP-1001`.
4. Choose `1` (**Auto-Assign**).
   - *Highlight:* The system automatically selects `AGT-02` (Mr. R. Verma - HOSTEL) because they are the department specialist with the fewest active complaints.
   - Status automatically transitions from `OPEN` $\to$ `ASSIGNED`.
   - Agent's active ticket workload increments to `1`.

---

### Step 4: Advance Ticket Status to IN PROGRESS
1. Enter `3` to **Advance Status to IN PROGRESS**.
2. Enter Complaint ID: `CMP-1001`.
3. Enter Progress Notes: `Electrician dispatched to Hostel Block B with replacement bearing`.
   - *Highlight:* Status updates to `IN_PROGRESS` and updates ticket's `updated_at` timestamp.

---

### Step 5: Change Priority with Justification Logging
1. Enter `4` to **Change Complaint Priority**.
2. Enter Complaint ID: `CMP-1001`.
3. Choose `4` (`CRITICAL`).
4. Enter Justification: `Electrical sparking creates fire hazard in dorm room`.
   - *Highlight:* Priority updated to `CRITICAL` with reason logged in the audit trail.

---

### Step 6: Demonstrate Escalation Management
1. **Manual Escalation:**
   - Enter `5` to **Escalate Complaint**.
   - Enter Complaint ID: `CMP-1001`.
   - Enter Escalation Reason: `Electrician reports spare motor out of stock; emergency procurement needed`.
   - *Highlight:* Status transitions to `ESCALATED`.
2. **Automated SLA Escalation Scanner:**
   - Enter `6` to **Run Automatic SLA Escalation Scanner**.
   - *Highlight:* Explains the 24-hour SLA rule. The scanner inspects all active tickets and detects SLA breaches based on calendar timestamp arithmetic.

---

### Step 7: Resolve the Complaint & Close with Customer Feedback
1. Enter `8` to **Resolve Complaint**.
2. Enter Complaint ID: `CMP-1001`.
3. Resolver Agent ID: Press `Enter` (accepts assigned agent `AGT-02`).
4. Enter Resolution Notes: `Fan motor replaced; wiring insulated and safety tested`.
   - *Highlight:* Status becomes `RESOLVED`. Agent `AGT-02` active ticket workload decrements back to `0`.
5. Enter `9` to **Close Complaint & Capture Customer Feedback**.
6. Enter Complaint ID: `CMP-1001`.
7. Rate Satisfaction: Enter `5` (5/5 Stars).
   - *Highlight:* Enforces business process rule: tickets cannot be closed unless first resolved. Records rating and moves status to `CLOSED`.

---

### Step 8: View Single Ticket Card & Chronological Audit Timeline
1. Enter `10` to **View Single Complaint Card & Audit Timeline**.
2. Enter Complaint ID: `CMP-1001`.
   - *Highlight:* Formatted card displaying Customer, Category, Priority, Resolution details, and Rating.
3. Choose `1` (`Yes` to view audit timeline).
   - *Highlight:* Chronological in-memory linked list showing exact timestamps, actions (`CREATED`, `ASSIGNED`, `IN_PROGRESS`, `PRIORITY`, `ESCALATED`, `RESOLVED`, `CLOSED`), actors, and remarks.

---

### Step 9: Multi-Criteria Search Queries
1. Enter `14` to open the **Search & Query Engine**.
2. Enter `2` (Search by Customer ID) $\to$ `CST-1003`. Shows all tickets by this customer.
3. Enter `3` (Search by Status) $\to$ Select `CLOSED`. Shows all closed tickets.
4. Enter `6` (Search by Keyword) $\to$ Type `fan`. Instantly finds matching complaints.
5. Enter `0` to return to Main Menu.

---

### Step 10: Executive Analytics & Summary Reports
1. Enter `16` to open **Summary Reports & Analytics**.
2. Enter `1` for **Executive KPI Dashboard**: Shows Total Tickets, Active vs Resolved counts, Resolution Rate (%), and Average Customer Rating.
3. Enter `2` for **Summary by Status**: Displays ASCII distribution bars for Open, Assigned, In Progress, Escalated, Resolved, Closed.
4. Enter `5` for **Agent Workload & Performance**: Displays active tickets, resolved counts, and average ratings per staff member.
5. Enter `0` to return to Main Menu.

---

### Step 11: File Persistence Verification
1. Enter `0` to **Exit System**.
   - *Highlight:* System confirms saving binary files (`data/complaints.dat`, `data/customers.dat`, `data/agents.dat`) and frees all memory.
2. Re-run `.\bin\complaint_system.exe`.
   - *Highlight:* Point out startup message: *"Loaded 1 complaint(s), 3 customer(s), 6 agent(s)"*. All state, ratings, and audit records were persistently restored from disk!
3. Exit cleanly by entering `0`.
