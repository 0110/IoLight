#!//bin/bash

if [ $# -ne 3 ]; then
	echo "Homie prefex and device index must be specified:"
	echo "$0 <mqtt host> <prefix> <device index>"
	echo "e.g."
	echo "$0 192.168.0.2 test/ MyDeviceId"
	exit 1
fi

mqttHost=$1
mqttPrefix=$2
homieId=$3

maxSteps=4

settingsFile=settings.json
if [ ! -f $settingsFile ]; then
	echo "$settingsFile missing"
	echo "check $settingsFile.example"
	exit 1
fi

echo "(1 / $maxSteps) Waiting ..."
mosquitto_sub -h $mqttHost -t "${mqttPrefix}${homieId}/#" -R -C 1
set -e
echo "(2 / $maxSteps) Waiting 30 seconds ..."
sleep 30
mosquitto_pub -h $mqttHost -t "${mqttPrefix}${homieId}/\$implementation/config/set" -f $settingsFile
echo "(3 / $maxSteps) Waiting for reboot ..."
sleep 1
mosquitto_sub -h $mqttHost -t "${mqttPrefix}${homieId}/#" -R -C 1
echo "(4 / $maxSteps) Alive"
exit 0
