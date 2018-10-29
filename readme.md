## Getting Start
### Compile
```bash
$>make
```
simple, isn't it? Now you will get a linux kernel module(netcco.ko) and a executable console(netccoctl)
### Install
```bash
$>sudo insmod netcco.ko
```
Now you already have it in system kernel, Let's check it out!
```bash
$>lsmod|grep netcco
netcco        10633   0
```
See?
### Usage
You can distort a ip/port pair using a new ip/port pair, like below
```bash
$>netccoctl 118.24.150.24 3306 118.24.150.22 3306
```
you will find out new request being sending to the '118.24.150.22:3306' which suppose to be sent to '118.24.150.24:3306' without change your configuration or reboot your applications.
