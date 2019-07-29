# ULSM
**U**ser-space **L**inux **S**ecurity **M**odule for fast research and development security concepts over LSM.

## What is LSM?
[LSM](https://en.wikipedia.org/wiki/Linux_Security_Modules) is a framework for developing security modules in Linux kernel. The framework enables such modules to register to actions with a callback function. When such actions occur, the function will be called and will be able to decide whether to allow or block the action. For example, an LSM can register a callback to be called on process execution (by implementing the [`bprm_check_security()`](https://elixir.bootlin.com/linux/v4.15/source/security/security.c#L339) function). When such process execution will occur, the callback function will be called, and the LSM can decide to block this execution.

![](images/morpheus.png?raw=true)

## What is ULSM?
ULSM is a lightweight  implementation of security module, that enables the developer to write the decision logic in user space. Each callback function extracts all the data from the kernel space and serializes it. Then it sends it to a user space process and waits for the verdict. The communication is performed with [netlink](http://man7.org/linux/man-pages/man7/netlink.7.html).
The user space process (`ulsmd`)  waits for new messages. When it receives a message it deserializes it, applies the logic and decides on verdict. It then sends this verdict back to the ULSM.
The ULSM receives the verdict and acts accordingly.
For example you can see the following diagram:

![](images/ulsm-diagram.png?raw=true)

## When to use
The main advantage of the ULSM is fast logic development. It enables the developer to write, test, and receive feedback  on its logic much quicker. This method is good especially for quick & dirty POCs, and for testing new security concepts.

## When not to use
The main disadvantages of the ULSM are:
Performance - Transferring each action to the user space increases the overhead of the LSM which hits performance.
Security - Adding serialization to the kernel is bad practice and increases the attack surface.
Synchronization - Need to be very careful with the operations you can perform from the user process in order to avoid deadlock.
Because of these disadvantages it is **not advised to use ULMS on production system**.

# Getting started
## Prerequisites
* Install [Cap'n PRoto](https://capnproto.org/install.html) by following the instructions on the repository.

* Install [c-capnproto](https://github.com/yeger00/c-capnproto) (C library for the Cap'n Proto) by following the instructions on the repository. Note: currently the kernel support is not in the official c-capnproto repository.

## Adding ULSM to your kernel
* Clone the code from the repository:
```shell
git clone https://github.com/argussecurity/ulsm.git
cd ulsm
```

* Copy the ULSM source code into the kernel source directory
```shell 
KERNEL_PATH=/path/to/kernel
cp -r security/ulsm $KERNEL_PATH/security/
```

* Copy the c-capnproto lib into the kernel source directory
```shell
C_CAPN_PATH=/path/to/c-capnproto
cp -r $C_CAPN_PATH/lib/ $KERNEL_PATH/security/ulsm/capn
```

* Patch the kernel security Makefile and Kconfig to enable ULSM complation. In the `security/` there is a patch file tested on `linux-4.19.60`. If you use this version, you can:
```shell
cd /path/to/linux-4.19.60/security/
patch -p0 < /path/to/ulsm-repo/security/Kconfig.patch
patch -p0 < /path/to/ulsm-repo/security/Makefile.patch
```
If not, you need to adapt for other version as well.

* Compile process.capnp
```shell
capnp compile -o$C_CAPN_PATH/capnpc-c -I $C_CAPN_PATH/compiler/ $KERNEL_PATH/security/ulsm/process.capnp
```

* Compile the kernel
Configure the kernel to use ULSM.
Compile the kernel.
Install the kernel.
On next boot you will start to see ULSM messages
```shell
dmesg | grep ulsm
```

## Running pyulsmd

* Install [pycapnp](https://github.com/capnproto/pycapnp) (Python bindings for the Cap'n Proto) by following the instructions on the repository.

* Run pyulsmd
```shell
cd pyulsmd
python pyulsmd.py
```

