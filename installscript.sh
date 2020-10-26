#!/bin/sh

echo "installing InfluxDB"
wget -qO- https://repos.influxdata.com/influxdb.key | sudo apt-key add -
source /etc/os-release
echo "deb https://repos.influxdata.com/debian $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/influxdb.list
apt-get update && sudo apt-get install influxdb
systemctl unmask influxdb.service
echo "influxdb -version"
systemctl start influxdb
echo "installing Grafana"
apt-get install -y apt-transport-https
apt-get install -y software-properties-common wget
wget -q -O - https://packages.grafana.com/gpg.key | sudo apt-key add -
echo "deb https://packages.grafana.com/oss/deb stable main" | sudo tee -a /etc/apt/sources.list.d/grafana.list
apt-get update
apt-get install grafana
echo "copying provision files"
cp -r provisioning/ /etc/grafana/
chgrp -R grafana /etc/grafana/provisioning/
cp logos/** /usr/share/grafana/public/img/
echo "configuring grafana-server to start at boot"
systemctl daemon-reload
systemctl enable grafana-server.service
systemctl start grafana-server
