#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/inet.h>

#define RATE_LIMIT_THRESHOLD 100    // Maximum allowed packets per minute
#define RATE_LIMIT_INTERVAL 60*HZ   // Time interval (1 minute in jiffies)

typedef struct {
    __be32 ip_addr;
    unsigned long last_packet_time;
    int packet_count;
    bool blocked;
} IPInfo;

IPInfo *ip_info_list = NULL;
int num_ips_tracked = 0;

// Function to check if an IP address is blocked
bool is_blocked(__be32 ip_addr) {
    int i;
    for (i = 0; i < num_ips_tracked; i++) {
        if (ip_info_list[i].ip_addr == ip_addr) {
            return ip_info_list[i].blocked;
        }
    }
    return false; // IP not found in the tracking list, so not blocked
}

// Function to add a new IP address to the tracking list
void add_ip(__be32 ip_addr) {
    IPInfo *new_ip_info = kmalloc(sizeof(IPInfo), GFP_KERNEL);
    if (new_ip_info == NULL) {
        printk(KERN_ERR "Failed to allocate memory for IPInfo\n");
        return;
    }
    new_ip_info->ip_addr = ip_addr;
    new_ip_info->last_packet_time = jiffies;
    new_ip_info->packet_count = 1;
    new_ip_info->blocked = false;

    // Append the new IPInfo to the tracking list
    if (ip_info_list == NULL) {
        ip_info_list = new_ip_info;
    } else {
        IPInfo *last_ip_info = ip_info_list;
        while (last_ip_info->next != NULL) {
            last_ip_info = last_ip_info->next;
        }
        last_ip_info->next = new_ip_info;
    }

    num_ips_tracked++;
}

// Function to update the packet count and last packet time for a tracked IP address
void update_ip(__be32 ip_addr) {
    int i;
    for (i = 0; i < num_ips_tracked; i++) {
        if (ip_info_list[i].ip_addr == ip_addr) {
            unsigned long now = jiffies;
            if (time_after(now, ip_info_list[i].last_packet_time + RATE_LIMIT_INTERVAL)) {
                // If the time interval has elapsed, reset the packet count
                ip_info_list[i].packet_count = 1;
                ip_info_list[i].last_packet_time = now;
            } else {
                ip_info_list[i].packet_count++;
                ip_info_list[i].last_packet_time = now;
                if (ip_info_list[i].packet_count >= RATE_LIMIT_THRESHOLD) {
                    // If the packet count exceeds the threshold, block the IP for a minute
                    ip_info_list[i].blocked = true;
                    mod_timer(&ip_info_list[i].unblock_timer, jiffies + 60*HZ);
                }
            }
            break;
        }
    }
}

// Function to unblock a blocked IP address after the blocking period has elapsed
void unblock_ip(unsigned long data) {
    IPInfo *ip_info = (IPInfo *)data;
    ip_info->blocked = false;
}

// Function to handle incoming packets and apply rate limiting
void handle_incoming_packet(__be32 src_ip) {
    if (is_blocked(src_ip)) {
        // If the source IP is already blocked, drop the packet
        printk(KERN_INFO "Blocked packet from IP: %pI4\n", &src_ip);
    } else {
        // If the source IP is not blocked, check if it's already tracked
        update_ip(src_ip);
        if (!is_tracked(src_ip)) {
            // If the IP is not tracked, add it to the tracking list
            add_ip(src_ip);
        }
        printk(KERN_INFO "Accepted packet from IP: %pI4\n", &src_ip);
    }
}
