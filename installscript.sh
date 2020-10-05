#!/bin/sh

echo "installing InfluxDB"
sudo apt-get update && sudo apt-get install influxdb
sudo systemctl unmask influxdb.service
echo "influxdb -version"
sudo systemctl start influxdb
echo "installing Grafana"
sudo apt-get install -y apt-transport-https
sudo apt-get install -y software-properties-common wget
wget -q -O - https://packages.grafana.com/gpg.key | sudo apt-key add -
echo "deb https://packages.grafana.com/enterprise/deb stable main" | sudo tee -a /etc/apt/sources.list.d/grafana.list
sudo apt-get update
sudo apt-get install grafana
echo "copying provision files"
sudo mkdir -p /etc/grafana/provisioning
sudo cp -r ./provisioning/* /etc/grafana/provisioning
echo "starting grafana server"
sudo systemctl daemon-reload
sudo systemctl start grafana-server
sudo systemctl status grafana-server
echo "configuring grafana-server to start at boot"
sudo systemctl enable grafana-server.service
/usr/bin/firefox&  localhost:3000
