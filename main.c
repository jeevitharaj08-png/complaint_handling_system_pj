#include "common.h"
#include "utils.h"
#include "customer.h"
#include "agent.h"
#include "complaint.h"
#include "storage.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>

static void print_startup_banner(void) {
    printf("=================================================================\n");
    printf("        COLLEGE COMPLAINT HANDLING SYSTEM (C11)                 \n");
    printf("   Defensive C Architecture | Persistent Binary & Audit Store   \n");
    printf("=================================================================\n");
}

int main(void) {
    print_startup_banner();

    /* 1. Initialize subsystem stores */
    storage_init_data_directory();
    customer_store_init();
    agent_store_init();

    ComplaintList complaints;
    complaint_list_init(&complaints);

    /* 2. Load persistent data from disk if available */
    printf("  [*] Loading persistent data stores from disk...\n");
    SystemResult load_res = storage_load_all(&complaints);
    if (load_res == SYS_OK) {
        printf("  [+] Loaded %zu complaint(s), %zu customer(s), %zu agent(s).\n",
               complaints.count, customer_get_count(), agent_get_count());
    } else {
        printf("  [!] Notice during data load: %s. Initializing fresh workspace.\n",
               utils_result_to_string(load_res));
    }

    /* Seed default college student if customer store is empty */
    if (customer_get_count() == 0) {
        char demo_cid[MAX_ID_LEN];
        customer_add("Rahul Sharma", "rahul.sharma@college.edu", "9876543210", "Hostel Block A, Room 204", demo_cid);
        customer_add("Priya Nair", "priya.nair@college.edu", "9123456780", "Hostel Block C, Room 108", demo_cid);
    }

    /* 3. Launch interactive main menu */
    ui_run_main_menu(&complaints);

    /* 4. Graceful Cleanup & Memory Deallocation (Zero Leaks) */
    complaint_list_cleanup(&complaints);
    customer_store_cleanup();
    agent_store_cleanup();

    return 0;
}
