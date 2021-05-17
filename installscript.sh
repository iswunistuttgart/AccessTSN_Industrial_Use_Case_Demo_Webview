#!/bin/bash

#// SPDX-License-Identifier: (MIT)
#/*
# * Copyright (c) 2020 Institute for Control Engineering of Machine Tools and Manufacturing Units, University of Stuttgart
# * Author: Philipp Neher <philipp.neher@isw.uni-stuttgart.de>
# *         Nico Brandt
# 
# * Permission is hereby granted, free of charge, to any person obtaining a copy
# * of this software and associated documentation files (the "Software"), to deal
# * in the Software without restriction, including without limitation the rights
# * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# * copies of the Software, and to permit persons to whom the Software is
# * furnished to do so, subject to the following conditions:
# *
# * The above copyright notice and this permission notice shall be included in all
# * copies or substantial portions of the Software.
# *
# * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
# * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# * SOFTWARE.
# */

echo "Building and installing of Webview Component of the AccessTSN Industrial Use Case Demo"
echo "Checking for prerequisites"

if ! command -v apt-get &> /dev/null
   then
       echo "Error: apt-get not found! Please install apt-get."
       exit 1
   else
       echo "apt-get found"
fi

if ! command -v wget &> /dev/null
   then
       echo "Error: wget not found! Please install wget."
       exit 1
   else
       echo "wget found"
fi

if ! command -v tee &> /dev/null
   then
       echo "Error: apt-get not found! Please install tee."
       exit 1
   else
       echo "tee found"
fi

if ! command -v gcc &> /dev/null
   then
       echo "Error: gcc not found! Please install gcc."
       exit 1
   else
       echo "gcc found"
fi

if ! command -v make &> /dev/null
   then
       echo "Error: make not found! Please install make."
       exit 1
   else
       echo "make found"
fi

if ! command -v dpkg &> /dev/null
   then
       echo "Error: dpkg not found! Please install dpkg."
       exit 1
   else
       echo "dpkg found"
fi

dpkg -s build-essential &> /dev/null
rtn=$?
if [ $rtn -eq 0 ]
   then
       echo "build-essential installed"
   else
       echo "Error: build-essential not installed! Please install build-essential."
       exit 1
fi

if ! command -v  lsb_release &> /dev/null
   then
       echo "Error: lsb_release not found! Please install package lsb-release."
       exit 1
   else
       echo "lsb_release found"
fi

echo "Cloning and updating submodules"
git submodule init
git submodule update

echo "Installing InfluxDB"
wget -q -O - https://repos.influxdata.com/influxdb.key | sudo apt-key add -
source /etc/os-release
echo "deb https://repos.influxdata.com/debian $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/influxdb.list
sudo apt-get update
sudo apt-get install influxdb
sudo systemctl unmask influxdb.service
if ! systemctl status influxdb &>/dev/null
    then
	echo "Error: Installation of InfluxDB failed."
	exit 1
    else
	echo "Installation of InfluxDB successful."
fi	

sudo systemctl start influxdb
echo "Installing Grafana"
sudo apt-get install -y apt-transport-https
sudo apt-get install -y software-properties-common wget
wget -q -O - https://packages.grafana.com/gpg.key | sudo apt-key add -
echo "deb https://packages.grafana.com/oss/deb stable main" | sudo tee -a /etc/apt/sources.list.d/grafana.list
sudo apt-get update
sudo apt-get install grafana
if ! systemctl status grafana-server &>/dev/null
    then
	echo "Error: Installation of Grafana failed."
	exit 1
    else
	echo "Installation of Grafana successful."
fi

echo "Copying provision files"
sudo cp -r provisioning/ /etc/grafana/
sudo chgrp -R grafana /etc/grafana/provisioning/
sudo cp logos/** /usr/share/grafana/public/img/
echo "Configuring grafana-server to start at boot"
sudo systemctl daemon-reload
sudo systemctl enable grafana-server.service
sudo systemctl start grafana-server

echo "Building AccessTSN Databaseconnector application using make"
cd src
make write2influxdb
cd ..

echo "Finished building application."
echo "The 'write2influxdb' application can be found in the 'src' subdirectory."
echo "It can be run with 'sudo write2influxdb -aio' (writing all variables)."
echo "For other options, see the CLI help using 'write2influxdb -h'."
exit 0

