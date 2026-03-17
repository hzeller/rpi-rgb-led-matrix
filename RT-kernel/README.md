
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
