# AccessTSN Industrial Use Case Demo - Webview
This is a demonstration to give an example on how to use Linux with the current support for Time Sensitive Networking (TSN) as an industrial endpoint. For this purpose the components of this demonstration form a converged network use case in an industrial setting. 

The Use Case Demonstration is aimed at the AccessTSN Endpoint Image and tested with it but should also work independently from the image.

This work is a result of the AccessTSN-Project. More information on the project as well as contact information are on the project's webpage: https://accesstsn.com

## The Industrial Use Case Demo - Webview Git-project

The AccessTSN Industrial Use Case Demo consists of multiple Git-projects, each holding the code to one component of the demonstration. This is the *Webview* project. This repository contains an interface to write data of a shared memory (defined in *mk_shminterface.h*) into InfluxDB. It also contains a connection to a Grafana installation and a dashboard to visualize the data written into InfluxDB. You can easily get a working stack by using the provided *installscript.sh* shell script. But you still need to write the data into the shared memory yourself.

The main repository for AccessTSN Industrial Use Case Demo can be found on Github: https://github.com/iswunistuttgart/AccessTSN_Industrial_Use_Case_Demo

# What to expect
Using the shell script for installation will add a repo and install a stable version of InfluxDB 1.8.3. Be aware that no user or password protection is included. InfluxDB should start at boot. If not, please refer to the official documentation of InfluxDB at [InfluxDB 1.8 documentation](https://docs.influxdata.com/influxdb/v1.8/). The script will also add a repo to Grafana and install it. After the installations are complete the files in the subdirectory _provisioning_ will be copied into the respective folder of the Grafana installation. After that Grafana will be configured to start at boot. Grafana will also be started after the installation.
Keep in mind that this repository does not contain code to write any data into the defined shared memory of *mk_shminterface.h*. You either need to develop your own code or get an example code.
Running the interface will create a database called _tsn_demo_ in InfluxDB. If it already exists all new data will be appended.

# Installation
Get the repository and all submodules with: 
`git clone --recurse-submodules <INSERT_REPO>`
Use the shellscript from within the repository:
`sudo ./installscript.sh`
Switch to the folder _src_ and compile _write2influxdb.cpp_.
`cd src/`
`make write2influxdb`

# Run Interface SHM to Influxdb
Run the compiled code with -h flag to see the switches for used variables:
`sudo ./write2influxdb -h`
or for use of all variables:
`sudo ./write2influxdb -aio`

# Use Grafana and verify the installation
Use your favorite web browser and navigate to _localhost:3000_
You will be prompted to sign into Grafana. Since it is the standard installation the user is _admin_ same as the password is _admin_
To be sure everything went as expected check the configuration menu by clicking the gear symbol on the left. You should see a single data source called _ISW_TSN_DEMO_. Open the configuration by clicking on it and scroll down to the bottom, hit the _test_ button and you should see a green field saying _Data source is working_. After verify the data source switch to _Dashboards -> Manage_ and select TSN_Demo. If data is written to the SHM now you will see the data in the dashboard.

# Tested Version
The initial released version is tested with 
- Debian GNU/Linux 10 (buster)
- InfluxDB version 1.6.4
- InfluxDB version 1.8.3
- Grafana v7.2.2 (ad9d408ac2)


# Nice to know
The dashboard is not working correctly with Grafana version 7.0.2
Sometimes the time is not shown correctly in the dashboard. If you encounter this you might want to set a timezone in Grafana.
If you want to access InfluxDB version 1.6.4 from the command line you need to install _influxdb-client_ 
The Data source and dashboard is loaded from provisioning files located at 
`/etc/grafana/provisioning`
If you make changes to the dashboard you need to save it as a new dashboard or change the .json file in the provisioning subdirectory _dashboards_
If you have no application to write into the SHM you might get an access error.
In test cases you needed to give sudo permission to the writer and reader of the SHM otherwise the applications were not working properly.