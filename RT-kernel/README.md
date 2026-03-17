
If you are suffering from random flickering because of other tasks run by your
linux kernel, you can install this RT kernel and kernel modules in your rPI image.

Note that installing a recent kernel (this kernel and module is 6.12.75-rt16-v8+)
on a very old rPi distro, userspace may not be compatible enough.

On my older dietpi distribution, the wifi firmware was too old to work with this
kernel.

I was able to fix it with 
but you could also try to get raspberry4_kernel_rt_trixie_120425_firmware.tar.gz
from https://mega.nz/file/rRcQDBqC#yhaByUa1z-TEmLc1joEF6QEZvDfirOIKrFf_yzoFOcA

this may also be enough:
apt-get update; apt-get install --reinstall firmware-brcm80211

Note that this RT kernel is aggressive. Once you start the RT code, on my Pi3 and Pi4,
it actually prevents the boot from completing if it hasn't yet (including networking),
so make sure networking is up and sshd running before you start RT code.

For more info and instructions, please review this bug:
https://github.com/hzeller/rpi-rgb-led-matrix/issues/1754#issuecomment-4014842065
including how to patch and build your own kernel:
https://github.com/hzeller/rpi-rgb-led-matrix/issues/1754#issuecomment-4017608624
Huge thanks to @kingdo9 for making all this happen.

Also, please note that RT does what it says, it will prevent the rest of linux
form running if needed, and it is normal to see output like this:
```
42.834070] rcu: INFO: rcu_preempt self-detected stall on CPU
42.834101] rcu: $3-....: (3 GPs behind) idle=7d5c/1/0x4000000000000000 softirq=0/0 f
491 rcuc=5301 jiffies (starved)
42.834124] rcu: t = 525theta jiffies g = 3129 alpha = 2711 ncpus=4) 105.845757] rcu: INFO: rcu_preempt self-detected stall on CPU
105.845787] rcu: $3-....: (3 GPs behind) idle=7d5c/1/0x4000000000000000 softirq=0/0 f
915 rcuc=21054 jiffies (starved)
105.845810] rcu: ( t = 21003 jiffies g = 3129 alpha = 4721 ncpus=4)
168.858199] rcu: INFO: rcu_preempt self-detected stall on CPU
168.858227] rcu: $3-....: (3 GPs behind) idle=7d5c/1/0x4000000000000000 softirq=0/0 f
7396 rcuc=36807 jiffies (starved)
168.858250] rcu: t = 36756 jiffies g = 3129 alpha = 6251 ncpus=4)
231.870579] rcu: INFO: rcu_preempt self-detected stall on CPU
231.870609] rcu: $3-....: (3 GPs behind) idle=7d5c/1/0x4000000000000000 softirq=0/0 f
4817 rcuc=52560 jiffies (starved)
231.870631] rcu: ( t = 52509 jiffies g = 3129 alpha = 7973 ncpus=4)
242.658727] INFO: task systemd: 1 blocked for more than 120 seconds.
242.658803] Not tainted 6.12.75-rt16-v8+ #1
242.658814] "echo > /proc/sys/kernel/hung_task_timeout_secs" disables this message.
242.659892] INFO: task (postboot): 513 blocked for more than 120 seconds.
242.659904] Not tainted 6.12.75-rt16-v8+ #1
242.659912] "echo > /proc/sys/kernel/hung_task_timeout_secs" disables this message.
```
