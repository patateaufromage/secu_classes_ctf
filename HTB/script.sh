#!/bin/bash
# This is the shebang line, specifying the script should be run with /bin/bash.

# Check for given arguments
if [ $# -eq 0 ]
then
	echo -e "You need to specify the target domain.\n" # If no arguments are given, print a usage message.
	echo -e "Usage:" 
	echo -e "\t$0 <domain>" # $0 represents the script name. This line guides the user on how to use the script.
	exit 1 # Exits the script with a status code of 1, indicating an error.
else
	domain=$1 # If an argument is given, it assigns the first argument to the variable 'domain'.
fi

# Function to identify network range for the specified IP addresses
function network_range {
	for ip in $ipaddr # Loop through each IP address stored in the 'ipaddr' variable.
	do
		netrange=$(whois $ip | grep "NetRange\|CIDR" | tee -a CIDR.txt) # Use whois to find network range, save and append it to 'CIDR.txt'.
		cidr=$(whois $ip | grep "CIDR" | awk '{print $2}') # Extracts the CIDR notation for the IP address.
		cidr_ips=$(prips $cidr) # Generates a list of all IPs within the CIDR range.
		echo -e "\nNetRange for $ip:" # Prints the net range for the current IP.
		echo -e "$netrange" # Prints the fetched network range.
	done
}

# Function to ping discovered IP addresses
function ping_host {
	hosts_up=0 # Counter for hosts that are up.
	hosts_total=0 # Counter for total hosts pinged.
	
	echo -e "\nPinging host(s):"
	for host in $cidr_ips # Loop through each IP in the CIDR IPs.
	do
		stat=1 # A flag to control the while loop below.
		while [ $stat -eq 1 ]
		do
			ping -c 2 $host > /dev/null 2>&1 # Ping the host silently, doing it twice.
			if [ $? -eq 0 ] # If ping succeeds, $? will be 0.
			then
				echo "$host is up." # Print that the host is up.
				((stat--)) # Decrease stat to exit the loop.
				((hosts_up++)) # Increment the counter for hosts that are up.
				((hosts_total++)) # Increment the total hosts counter.
			else
				echo "$host is down." # Print that the host is down.
				((stat--)) # Decrease stat to exit the loop.
				((hosts_total++)) # Increment the total hosts counter.
			fi
		done
	done
	
	echo -e "\n$hosts_up out of $hosts_total hosts are up." # Print summary of hosts that are up vs. total.
}

# Identify IP address of the specified domain
hosts=$(host $domain | grep "has address" | cut -d" " -f4 | tee discovered_hosts.txt) # Use 'host' command to find IPs of the domain, extract IPs, and save them to 'discovered_hosts.txt'.

echo -e "Discovered IP address:\n$hosts\n" # Print the discovered IP addresses.
ipaddr=$(host $domain | grep "has address" | cut -d" " -f4 | tr "\n" " ") # Same as above but converts newline characters to spaces for the 'ipaddr' variable.

# Available options
echo -e "Additional options available:" # Inform user of the next steps available.
echo -e "\t1) Identify the corresponding network range of target domain."
echo -e "\t2) Ping discovered hosts."
echo -e "\t3) All checks."
echo -e "\t*) Exit.\n"

read -p "Select your option: " opt # Prompt the user to select an option.

case $opt in
	"1") network_range ;; # If option 1 is selected, call the network_range function.
	"2") ping_host ;; # If option 2 is selected, call the ping_host function.
	"3") network_range && ping_host ;; # If option 3 is selected, perform both network range identification and pinging hosts.
	"*") exit 0 ;; # Any other input will exit the script.
esac

