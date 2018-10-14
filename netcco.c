#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/inet.h>
#include <linux/netdevice.h>
#include <net/tcp.h>
#include <net/ip.h>
#include <net/checksum.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include "nc_def.h"


#define NC_UNMATCHED   (0)
#define NC_MATCHED     (1)
#define MAX_DISTORTS   (100)
static int                   distorts_len;
static struct nc_distort     distorts[MAX_DISTORTS];
static struct nc_distort     *distort_buf;
static struct nf_hook_ops    i_hook;
static struct nf_hook_ops    o_hook;
static struct iphdr          *ip_header;
static struct tcphdr         *tcp_header;
static struct proc_dir_entry *pde;
static struct file_operations proc_ops;
static unsigned int i;

static void update_chksum(struct sk_buff *skb, struct iphdr *iph, struct tcphdr *tcp)
{
    //ip
    iph->check = 0;        
    iph->check = ip_fast_csum(iph,iph->ihl);

    //tcp
    tcp->check = 0;
    tcp->check = csum_tcpudp_magic(iph->saddr, iph->daddr,  
                                   ntohs(iph->tot_len)-iph->ihl*4, IPPROTO_TCP,  
                                   csum_partial(tcp, ntohs(iph->tot_len)-iph->ihl*4, 0));

    //checksum
    skb->ip_summed = CHECKSUM_NONE;
}

static int block_old_source(const struct nc_distort distort)
{
    if (ip_header->saddr==distort.old_addr&&
        tcp_header->source==distort.old_port) {
           return NC_MATCHED;
    }
    return NC_UNMATCHED;
}

static int incoming_distort_source(const struct nc_distort distort)
{
    if (ip_header->saddr==distort.new_addr&&
        tcp_header->source==distort.new_port) {
           ip_header->saddr = distort.old_addr;
           tcp_header->source = distort.old_port;
           
           return NC_MATCHED;
    }
    return NC_UNMATCHED;
}

static int outgoing_distort_dest(const struct nc_distort distort)
{
    if (ip_header->daddr==distort.old_addr&&
        tcp_header->dest==distort.old_port) {
        ip_header->daddr = distort.new_addr;
        tcp_header->dest = distort.new_port;
        return NC_MATCHED;
    }
    return NC_UNMATCHED;
}

static unsigned int _hook_incoming(void* priv, 
                                   struct sk_buff* skb, 
                                   const struct nf_hook_state* st)
{
    skb_linearize(skb);
    ip_header  = ip_hdr(skb);
	tcp_header = tcp_hdr(skb);
	// struct udphdr *udp_header 		=	udp_hdr(skb);
    if (ip_header->protocol==IPPROTO_TCP) {
        for (i = 0;i<distorts_len;i++) {
            if (block_old_source(distorts[i])) {
                return NF_DROP;
            }
            if (incoming_distort_source(distorts[i])) {
                update_chksum(skb, ip_header, tcp_header);
                return NF_ACCEPT;
            }
        }
    }
    return NF_ACCEPT;
}

static unsigned int _hook_outgoing(void* priv,
                                   struct sk_buff* skb,
                                   const struct nf_hook_state* st)
{
    skb_linearize(skb);
    ip_header  = ip_hdr(skb);
	tcp_header = tcp_hdr(skb);
    
    
    if (ip_header->protocol==IPPROTO_TCP) {
        for (i=0;i<distorts_len;i++) {
            if (outgoing_distort_dest(distorts[i])) {
                update_chksum(skb, ip_header, tcp_header);
                return NF_ACCEPT;
            }
        }
    }
    return NF_ACCEPT;
}

static void install_hooks(void){
    i_hook.hook      = _hook_incoming;
    i_hook.hooknum   = NF_INET_PRE_ROUTING;
    i_hook.pf        = PF_INET;
    i_hook.priority  = NF_IP_PRI_FIRST;
    nf_register_net_hook(&init_net, &i_hook);
    
    o_hook.hook      = _hook_outgoing;
    o_hook.hooknum   = NF_INET_POST_ROUTING;
    o_hook.pf        = PF_INET;
    o_hook.priority  = NF_IP_PRI_FIRST;
    nf_register_net_hook(&init_net, &o_hook);
}

static void uninstall_hooks(void){
    nf_unregister_net_hook(&init_net, &i_hook);
    nf_unregister_net_hook(&init_net, &o_hook);
}

static ssize_t proc_op_write(struct file *filp,
                             const char __user *buffer,
                             size_t len,
                             loff_t* offset)
{
    distort_buf = (struct nc_distort*)vmalloc(sizeof(struct nc_distort));
    if (!distort_buf) {
        return -ENOMEM;
    }
    if(copy_from_user(distort_buf, buffer, len)){
        vfree(distort_buf);
        return -EFAULT;
    }
    distorts[distorts_len].old_addr = distort_buf->old_addr;
    distorts[distorts_len].old_port = distort_buf->old_port;
    distorts[distorts_len].new_addr = distort_buf->new_addr;
    distorts[distorts_len].new_port = distort_buf->new_port;
    distorts_len++;
    vfree(distort_buf);
    return len;
}

static void set_proc_ops(void){
    proc_ops.owner = THIS_MODULE;
    proc_ops.write = proc_op_write;
    pde = proc_create(procfs_name, 0666, NULL, &proc_ops);
    if (!pde) {
        printk(KERN_ERR "proc create failed!");
        proc_remove(pde);
    }
}

static void unset_proc_ops(void){
    if(pde){
        proc_remove(pde);
    }
}

static int __init ncco_init(void){
    
    install_hooks();
    set_proc_ops();
    return 0;
}

static void __exit ncco_exit(void){
    uninstall_hooks();
    unset_proc_ops();
}

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("netcco Loadable Kernel Module");
MODULE_AUTHOR("Zhideng");

module_init(ncco_init);
module_exit(ncco_exit);

