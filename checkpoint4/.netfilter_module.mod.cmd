cmd_/home/hodaya/Desktop/checkpoint4/netfilter_module.mod := printf '%s\n'   netfilter_module.o | awk '!x[$$0]++ { print("/home/hodaya/Desktop/checkpoint4/"$$0) }' > /home/hodaya/Desktop/checkpoint4/netfilter_module.mod
