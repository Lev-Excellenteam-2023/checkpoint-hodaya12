#include <linux/jiffies.h>
#include <linux/slab.h>
#include <linux/inet.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>

static struct nf_hook_ops nfho;

// Forward declaration for the unblock_ip function
void unblock_ip(struct timer_list *timer);

void handle_incoming_packet(__be32 src_ip);










// Forward declaration for the is_blocked function
bool is_blocked(__be32 ip_addr);

// ... (rest of the code)

unsigned int hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *ip_header;
    ip_header = ip_hdr(skb);

    // Convert the unsigned int to __be32 using htonl()
    __be32 src_ip = htonl(ip_header->saddr);

    // Check if the source IP is blocked
    if (is_blocked(src_ip)) {
        // If the source IP is blocked, drop the packet
        printk(KERN_INFO "Blocked packet from IP: %pI4\n", &src_ip);
        return NF_DROP;
    }

    handle_incoming_packet(src_ip);
    
    return NF_ACCEPT;
}

int my_module_init(void)
{
    nfho.hook = hook_func;
    nfho.hooknum = NF_INET_PRE_ROUTING;
    nfho.pf = PF_INET;
    nfho.priority = NF_IP_PRI_FIRST;

    nf_register_net_hook(&init_net, &nfho);

    return 0;
}

void my_module_exit(void)
{
    nf_unregister_net_hook(&init_net, &nfho);
   
}

module_init(my_module_init);
module_exit(my_module_exit);


MODULE_DESCRIPTION("Netfilter module to log source and destination IP addresses");












#define RATE_LIMIT_THRESHOLD 2    // Maximum allowed packets per minute
#define RATE_LIMIT_INTERVAL 60*HZ   // Time interval (1 minute in jiffies)

typedef struct IPInfo {
    __be32 ip_addr;
    unsigned long last_packet_time;
    int packet_count;
    bool blocked;
    struct timer_list unblock_timer;  // Timer to unblock the IP address
    struct IPInfo* next;  // Pointer to the next IPInfo struct in the linked list
} IPInfo;

IPInfo *ip_info_list = NULL;
int num_ips_tracked = 0;
bool is_tracked(__be32 ip_addr) {
    IPInfo *p = ip_info_list;
    if (p == NULL)
        return false;
    while (p != NULL) {
        if (p->ip_addr == ip_addr)
            return true;
        p = p->next;
    }
    return false;
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

    // Initialize the timer for this IP address
    timer_setup(&new_ip_info->unblock_timer, unblock_ip, 0); // The third argument is flags, set to 0 here.

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

// Function to unblock a blocked IP address after the blocking period has elapsed
void unblock_ip(struct timer_list *timer) {
    IPInfo *ip_info = from_timer(ip_info, timer, unblock_timer);
    ip_info->blocked = false;
}



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
MODULE_LICENSE("GPL v2");