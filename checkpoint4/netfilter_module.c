#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>

static struct nf_hook_ops nfho;

unsigned int hook_func(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct iphdr *ip_header;
    ip_header = ip_hdr(skb);
    
    if (ip_header->protocol == IPPROTO_TCP || ip_header->protocol == IPPROTO_UDP) {
        printk(KERN_INFO "Incoming Packet - Source IP: %pI4, Destination IP: %pI4\n",
               &ip_header->saddr, &ip_header->daddr);
    }

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
    kfree(nfho);
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Netfilter module to log source and destination IP addresses");
