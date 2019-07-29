subdir-ccflags-y += -I$(src)/capn -Wno-unused
CFILES=\
       hashtable.c \
       netlink-protocol.c \
       ulsm.c \
       capn/capn.c \
       capn/capn-malloc.c \
       capn/capn-stream.c \
       process.capnp.c
OBJS = $(CFILES:.c=.o)
obj-$(CONFIG_SECURITY_ULSM) += ulsm.o
ulsm-y += $(OBJS)
