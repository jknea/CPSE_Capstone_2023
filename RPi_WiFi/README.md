## RPi WiFi Access Point Setup  
This could be done in fewer steps, but the way I initially set it up was to first establish the RPi as a WiFi access point and then configure the AP to also accept Ethernet traffic.

### Install required packages  
**$ sudo apt-get -y install hostapd dnsmasq**  
Ignore the wireless interface  
**$ sudo nano /etc/dhcpcd.conf**  
### Set a static IP  
At the bottom of the file, add:	  
denyinterfaces wlan0  
**$ sudo nano /etc/network/interfaces**  
At the bottom of the file, add:	  
auto lo  
iface lo inet loopback  

auto eth0  
iface eth0 inet dhcp  

allow-hotplug wlan0  
iface wlan0 inet static  
    address 192.168.5.1  
    netmask 255.255.255.0  
    network 192.168.5.0  
    broadcast 192.168.5.255

### Configure HostAPD to broadcast custom SSID and allow WiFi connections on a certain channel  
**$ sudo nano /etc/hostapd/hostapd.conf**  
Enter the following into the file:  
interface=wlan0  
//	driver=nl80211  
ssid=MyPiAP  
//	hw_mode=g  
//	channel=6  
channel=9  
//	ieee80211n=1  
//	wmm_enabled=1  
//	ht_capab=[HT40][SHORT-GI-20][DSSS_CCK-40]  
//	macaddr_acl=0  
//	auth_algs=1  
//	ignore_broadcast_ssid=0  
wpa=2  
wpa_key_mgmt=WPA-PSK  
wpa_passphrase=raspberry  
rsn_pairwise=CCMP  

### Inform HostAPD of location of the previously created configuration file  
**$ sudo nano /etc/default/hostapd**  
Find the line #DAEMON_CONF="" and replace it with:  DAEMON_CONF="/etc/hostapd/hostapd.conf"  

### Configure DNSMasq  
Backup the old configuration file:  
**$ sudo mv /etc/dnsmasq.conf /etc/dnsmasq.conf.bak**  
Create the new configuration file:  
**$ sudo nano /etc/dnsmasq.conf**  
Enter the following in the blank file:  
interface=wlan0   
listen-address=192.168.5.1  
bind-interfaces   
server=8.8.8.8  
domain-needed  
bogus-priv  
dhcp-range=192.168.5.100,192.168.5.200,24h  

### Restart the Raspberry Pi
**$ sudo reboot**


### Configure ETHERNET (added things below where defined from AP setup)  
**$ sudo nano /etc/network/interfaces**  
Add the following at the bottom:  
allow-hotplug eth0   
iface eth0 inet static   
address 192.168.2.1   
netmask 255.255.255.0   
network 192.168.2.0   
broadcast 192.168.2.255  

**$ sudo nano /etc/dnsmasq.conf**  
Add the following to the bottom:  
interface=eth0 # Use interface eth0     
listen-address=192.168.2.1 # listen on    
### Bind to the interface to make sure we aren't sending things elsewhere  
bind-interfaces  
server=8.8.8.8 # Forward DNS requests to Google DNS  
domain-needed # Don't forward short names  
bogus-priv  # Never forward addresses in the non-routed address spaces.   
dhcp-range=192.168.2.2,192.168.2.100,12h  # Assign IP addresses between 192.168.2.2 and 192.168.2.100 with a 12 hour lease time   

**$ sudo nano /etc/sysctl.conf**  

Remove the # from the beginning of the line containing: net.ipv4.ip_forward=1  

**$ sudo sh -c "echo 1 > /proc/sys/net/ipv4/ip_forward"**  

**$ sudo iptables -t nat -A POSTROUTING -o wlan0 -j MASQUERADE**  

**$ sudo iptables -A FORWARD -i wlan0 -o eth0 -m state --state RELATED,ESTABLISHED -j ACCEPT**  

**$ sudo iptables -A FORWARD -i eth0 -o wlan0 -j ACCEPT**  

**$ sudo sh -c "iptables-save > /etc/iptables.ipv4.nat"**  

**$ sudo nano /etc/rc.local**  

Just above the line exit 0, add the following line: 
iptables-restore < /etc/iptables.ipv4.nat

### Restart the Raspberry Pi
**$ sudo reboot**


### Sources:  
[Setting up a Raspberry Pi 3 as an Access Point - SparkFun Learn](https://learn.sparkfun.com/tutorials/setting-up-a-raspberry-pi-3-as-an-access-point/all)  

[raspbian - Sharing the Pi's WiFi connection through the Ethernet port - Raspberry Pi Stack Exchange](https://raspberrypi.stackexchange.com/questions/48307/sharing-the-pis-wifi-connection-through-the-ethernet-port)  
