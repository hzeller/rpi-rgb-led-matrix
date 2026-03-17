# Real Time Kernel

If you are suffering from random flickering on your display, you can install this RT kernel and kernel modules in your rPI image.

First perform the below **cmdline.txt** modification, this also helps with intermittent flicker especially when combined with the real time kernel as guided below. Apply this regardless of using RT kernel or not, as it helps with interrupts on Core 3 as you can see when doing  `cat /proc/interrupts`
```
nano /boot/firmware/cmdline.txt

# Add the below to the end of the long string.
 isolcpus=domain,managed_irq,3 nohz_full=3 rcu_nocbs=3 irqaffinity=0,1,2
```
---

[Option 1 - Download Preinstalled Image - Pi 4 - Trixie 64-bit Lite](#option-1---download-preinstalled-image---pi4---trixie-64-bit-lite)

[Option 2 - Copy RT Kernel To Your Current Raspberry OS installation](#option-2---copy-rt-kernel-to-your-current-raspberry-os-installation)

[Option 3 - Compile and Apply Your Own Raspberry Kernel](#option-3---compile-and-install-your-own-raspberry-kernel)

---
<br>

## Option 1 - Download Preinstalled Image - Pi4 - Trixie 64-bit Lite

<a href="https://mega.nz/file/Hc8wiaJK#JY78Dx60iUMU_hcC7IG0-yVFvBAFFW-4MQ_A3D9nsRM">Raspberry Pi 4 Lite Trixie 64-Bit - RT Kernel Preinstalled image</a>

<img alt="Image" src="./img/kernel_guide_1.png" /><br>
<img alt="Image" src="./img/kernel_guide_2.png" />

Additional mods already done to the image.

CTRL+S , CTRL+X to Save and Exit from Nano

```
# Disable Audio for PWM to work
nano /boot/firmware/config.txt

dtparam=audio=off
dtoverlay=vc4-kms-v3d,noaudio,disable-bt

nano /etc/modprobe.d/blacklist-audio.conf
blacklist snd_bcm2835

# Disable Wi-Fi Power Saving
nano /etc/rc.local

#!/bin/sh -e
/sbin/iwconfig wlan0 power off
exit 0

chmod +x /etc/rc.local
```
<br>

---
## Option 2 - Copy RT Kernel To Your Current Raspberry OS installation

**This precompiled kernel is for Raspberry Pi 4 64-bit. If you need anything different you will need to [compile your own](#option-3---compile-and-install-your-own-raspberry-kernel)**

Download and copy the kernel files to your Raspberry Pi in the /tmp directory.

The kernel version compiled is 6.12.75-rt16-v8+
So its best your OS image is already close to it.

[raspberry4_kernel_rt_trixie_120425_boot.tar.gz](https://github.com/hzeller/rpi-rgb-led-matrix/blob/master/RT-kernel/raspberry4_kernel_rt_trixie_120425_boot.tar.gz)
[raspberry4_kernel_rt_trixie_120425_root.tar.gz](https://github.com/hzeller/rpi-rgb-led-matrix/blob/master/RT-kernel/raspberry4_kernel_rt_trixie_120425_root.tar.gz)


```
tar -xzf /tmp/raspberry4_kernel_rt_trixie_120425_boot.tar.gz -C /boot/firmware
tar -xzf /tmp/raspberry4_kernel_rt_trixie_120425_root.tar.gz -C /
```

Reboot and check your kernel reads
```
uname -a
Linux raspberry 6.12.75-rt16-v8+ #1 SMP PREEMPT_RT
```
###########################################################

**Optionally**, if your original kernel was older, you may need to perform the additional step to match the firmware. If you notice Wi-Fi is not working etc.

You may only need the below to fix it 
```
apt-get update; apt-get install --reinstall firmware-brcm80211
```
<br>

Alternatively extract it from below.
Kernel firmware
https://mega.nz/file/rRcQDBqC#yhaByUa1z-TEmLc1joEF6QEZvDfirOIKrFf_yzoFOcA
```
tar -xzf /tmp/raspberry4_kernel_rt_trixie_120425_firmware.tar.gz -C /
```
<br>
Depending on what your original kernel/os image was, you may see some dmesg prompts like the below which should be fine.

```
42.834070] rcu: INFO: rcu_preempt self-detected stall on CPU
42.834101] rcu: $3-....: (3 GPs behind) idle=7d5c/1/0x4000000000000000 softirq=0/0 f
491 rcuc=5301 jiffies (starved)
42.834124] rcu: t = 525theta jiffies g = 3129 alpha = 2711 ncpus=4) 105.845757] rcu: INFO: rcu_preempt self-detected stall on CPU
105.845787] rcu: $3-....: (3 GPs behind) idle=7d5c/1/0x4000000000000000 softirq=0/0 f
915 rcuc=21054 jiffies (starved)
```
<br>


---
## Option 3 - Compile and Install Your Own Raspberry Kernel
 Install and start Raspberry OS, I'm using Trixie Lite 64-bit
 Find the kernel version of your current Raspberry Pi install.

```
uname -a
```
<span style="color:darkred;font-weight:bold">
 Using a Desktop Ubuntu Linux system, currently using Ubuntu 24.0 - To cross compile

</span>

```
apt install -y bc bison flex libssl-dev make libc6-dev libncurses-dev libelf-dev crossbuild-essential-arm64 git
cd /home/user/
mkdir kernel
cd kernel
git clone https://github.com/raspberrypi/linux.git
cd linux
```
 <br>

 **OPTIONAL** - Below will show the current kernel version currently checked out. Run the below to change kernel version by using git checkout. To find kernel version branches available, navigate to https://github.com/raspberrypi/linux
```
head -n 4 Makefile
git checkout rpi-6.12.y
```
<img width="532" height="446" alt="Image" src="https://github.com/user-attachments/assets/2d178e5f-a848-4b8a-a33a-53243e708230" /><br><br><br>
 Go to the below and find your nearest RT kernel patch example**
https://www.kernel.org/pub/linux/kernel/projects/rt

 Right click and copy the address of the patch.gz e.g

<img width="599" height="313" alt="Image" src="https://github.com/user-attachments/assets/d73ef3ba-9315-4582-9c79-eb0ac6346dd0" /><br><br><br>

```
wget https://www.kernel.org/pub/linux/kernel/projects/rt/6.12/patch-6.12.74-rt16.patch.gz
gunzip patch*.gz
cat patch*.patch | patch -p1
patch -p1 < *.patch
```

 **Make sure the patch fully succeeds example, and no failures.**
`Hunk #8 succeeded at 3626 (offset 9 lines).`

 **Set the Kernel variable where kernel8 as being  Pi 3/4/Zero2W | Other references link
https://www.raspberrypi.com/documentation/computers/linux_kernel.html**

```
KERNEL=kernel8
```

 **Make default config, my example is Pi 4**
```
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- bcm2711_defconfig
```

 **Other Config examples below.**

 **64-bit (ARCH=arm64)**
 
 Pi 3 / 3+ / CM3 / CM3+ / Zero 2 W / Pi 4 / Pi 400 / CM4 / CM4S -  `bcm2711_defconfig`
 
 Pi 5 / Pi 500 / CM5 - `bcm2712_defconfig`

 **32-bit (ARCH=arm)**

 Pi 1 / CM1 / Zero / Zero W / Pi 2 - `bcmrpi_defconfig`

 Pi 3 / 3+ / CM3 / CM3+ / Zero 2 W - `bcm2709_defconfig`

 **Note: Raspberry Pi OS 32-bit on Pi 4 class devices normally uses a 64-bit kernel by default; building a true 32-bit kernel for those needs ARCH=arm and extra boot config.**

<br>

 **Edit Kernel options**
```
make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- menuconfig
```
```Space to select options.
General Setup > Timers Subsystem > Timer tick handling > Full dynticks system (tickless)
General Setup > Fully Preemptible Kernel (Real-Time)
Save
Exit
```
<img width="647" height="214" alt="Image" src="https://github.com/user-attachments/assets/1d073d66-f035-4d8c-8bd4-7a0c02ee7357" /><br>
<img width="511" height="76" alt="Image" src="https://github.com/user-attachments/assets/11d5becb-6c06-43e3-9186-836fd1b12785" /><br><br>

```
make -j"$(nproc)" ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- Image.gz modules dtbs
```

 **Now we copy the relevant files to the Raspberry OS sdcard.**
 **Use SD card reader to attach sdcard to Ubuntu system.**
 **In the console, find the mountpoints of the sdcard.**
```
mount | grep /dev/sd
```
 **Example**
```
/media/user/bootfs
/media/user/rootfs
```

 **Making sure you are still in the current working directory e.g**
```
cd /home/user/kernel/linux
```

 **Backup existing Kernel and related files**
```
cp /media/user/bootfs/${KERNEL}.img /media/user/bootfs/${KERNEL}.bak
mkdir -p /media/user/bootfs/dtbbak
mkdir -p /media/user/bootfs/overlays/dtboverlaysbak
cp /media/user/bootfs/*.dtb /media/user/bootfs/dtbbak
cp /media/user/bootfs/overlays/*.dtb* /media/user/bootfs/overlays/dtboverlaysbak
```

 **Copy the RT Kernel to the sdcard**
```
cp arch/arm64/boot/Image.gz /media/user/bootfs/${KERNEL}.img
cp arch/arm64/boot/dts/broadcom/*.dtb /media/user/bootfs
cp arch/arm64/boot/dts/overlays/*.dtb* /media/user/bootfs/overlays
sudo make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- INSTALL_MOD_PATH=/media/user/rootfs modules_install
```

 **Add the Kernel parameters for RT at the end of the line - CTRL+S , CTRL+X to save in nano**
```
nano /media/user/bootfs/cmdline.txt

# Add this to the end of the line
 isolcpus=domain,managed_irq,3 nohz_full=3 rcu_nocbs=3 irqaffinity=0,1,2

```

 **Put the sdcard back into the Pi and boot**
 **Confirm Real Time kernel is now installed**

```
uname -a
```

---

Original discussion thread
https://github.com/hzeller/rpi-rgb-led-matrix/issues/1754

