#!/bin/bash

powered_state=0
line_count=0

function force_modem_online() {
  sleep 2
  dbus-send --system --type=method_call --print-reply \
    --dest=org.ofono /ril_0 org.ofono.Modem.SetProperty \
    string:"Online" variant:boolean:true
  powered_state=0
  line_count=0
}

dbus-monitor --system "type='signal',interface='org.ofono.Modem',member='PropertyChanged'" |
while read -r line; do
  echo $line | grep "Powered" && powered_state=1 && line_count=$(($line_count + 1))
  [ $powered_state == 1 ] && [ $line_count == 1 ] && echo $line | grep "boolean true" && force_modem_online
done
