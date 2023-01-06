#!//bin/bash

if [ $# -lt 3 ]; then
	echo "Homie prefex and device index must be specified:"
	echo "$0 <mqtt host> <prefix> <device id> (mqtt user)"
	echo "e.g."
	echo "$0 192.168.0.2 test/ MyDeviceId"
	echo "or"
	echo "$0 192.168.0.2 test/ MyDeviceId user1"
	echo "Password of user is requested inside this script"
	exit 1
fi

mqttHost=$1
mqttPrefix=$2
homieId=$3
firmwareFile=../.pio/build/nodemcuv2/firmware.bin

if [ ! -f $firmwareFile ]; then
	echo "the script $0 must be started in host/ sub directory"
	echo "and firmware must be compiled"
	exit 2
fi

# check for required tools
command -v mosquitto_sub >> /dev/null
if [ $? -ne 0 ]; then
 echo "mosquitto_sub missing"
 echo ""
 echo "install with .e.g. apt : apt install mosquitto-clients"
 exit 3
fi

PYTHONCMD=python
command -v $PYTHONCMD >> /dev/null
if [ $? -ne 0 ]; then
	PYTHONCMD=python3
	command -v $PYTHONCMD >> /dev/null
	if [ $? -ne 0 ]; then
		echo "No python found:"
		whereis python
		whereis python3
		exit 4
	fi
fi

# Handle MQTT authentication
MOSQUITTO_AUTH=""
OTA_AUTH=""
if [ $# -eq 4 ]; then
 mqttUser=$4
 echo "Enter password for $mqttUser on $mqttHost"
 read mqttPasswd
 MOSQUITTO_AUTH=" -u $mqttUser -P $mqttPasswd" 
 OTA_AUTH=" -u $mqttUser -d $mqttPasswd"
fi

deviceState=$(mosquitto_sub -h $mqttHost $MOSQUITTO_AUTH -t "${mqttPrefix}${homieId}/\$state" -C 1)
if [ $deviceState != "ready" ]; then
 echo "Waiting for $homieId ..."
 mosquitto_sub -h $mqttHost $MOSQUITTO_AUTH -t "${mqttPrefix}${homieId}/#" -R -C 1
fi

$PYTHONCMD ota_updater.py -l $mqttHost $OTA_AUTH -t "$mqttPrefix" -i "$homieId" $firmwareFile

exit 0
