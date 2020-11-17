// SPDX-License-Identifier: (MIT)
/*
 * Copyright (c) 2020 Institute for Control Engineering of Machine Tools and Manufacturing Units, University of Stuttgart
 * Author: Philipp Neher <philipp.neher@isw.uni-stuttgart.de>
 *         Nico Brandt
 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "../influxdb-cpp/influxdb.hpp"
#include "../accesstsn_demoapps_common/mk_shminterface.h"
#include "mk_shminterface.h"
#include <signal.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <iostream>
#include <chrono>

uint8_t run = 1;

struct demoreader_t
{
	struct mk_mainoutput *mainout;
	struct mk_maininput *mainin;
	struct mk_additionaloutput *addout;
	uint32_t period;
	bool flagmainout;
	bool flagmainin;
	bool flagaddout;
};

/**
 * @brief creates database in InfluxDB
 *
 * creates database "dbname" in InfluxDB living under "ip:port" with "user" and "pwd"
 *
 * @pre InfluxDB installed \link https://portal.influxdata.com/downloads tested with v1.8.3\endlink
 * @param[in] ip IPv4 of host for InfluxDB
 * @param[in] port Port of host for InfluxDB
 * @param[in] dbname Name of database in InfluxDB
 * @param[in] user Username to access InfluxDB
 * @param[in] pwd Password to access InfluxDB
 * @return influxdb_cpp::server_info filled with param[in]
 */
influxdb_cpp::server_info initDB(std::string ip, int port, std::string dbname,
		std::string user, std::string pwd)
{

	std::cout << "initializing host: " << ip << ":" << port
			<< " with database: " << dbname << std::endl;
	influxdb_cpp::server_info si(ip, port, dbname, user, pwd);
	std::string resp;//	needed for method influxdb_cpp::create_db(resp, db_name, si)
	influxdb_cpp::create_db(resp, dbname, si);

	return si;
}

/* signal handler */
void sigfunc(int sig)
{
	switch (sig)
	{
	case SIGINT:
		if (run)
			run = 0;
		else
			exit(0);
		break;
	case SIGTERM:
		run = 0;
		break;
	}
}

/* Print usage message */
static void usage(char *appname)
{
	fprintf(stderr,
			"\n"
					"Usage: %s [options]\n"
					" -o            Reads main output variables from control\n"
					" -i            Reads main input variables to control\n"
					" -a            Reads additional output variables from control\n"
					" -t [value]    Specifies update-period in milliseconds. Default 10 seconds.\n"
					" -h            Prints this help message and exits\n"
					"\n", appname);
}

/* Evaluate CLI-parameters */
void evalCLI(int argc, char *argv[0], struct demoreader_t *reader)
{
	int c;
	char *appname = strrchr(argv[0], '/');
	appname = appname ? 1 + appname : argv[0];
	while (EOF != (c = getopt(argc, argv, "oiaht:")))
	{
		switch (c)
		{
		case 'o':
			(*reader).flagmainout = true;
			break;
		case 'i':
			(*reader).flagmainin = true;
			break;
		case 'a':
			(*reader).flagaddout = true;
			break;
		case 't':
			(*reader).period = atoi(optarg) * 1000;
			break;
		case 'h':
		default:
			usage(appname);
			exit(0);
			break;
		}
	}
	if (((*reader).flagmainout == false) && ((*reader).flagmainin == false)
			&& ((*reader).flagaddout) == false)
	{
		printf("At minium, one block of variables needs to be selected\n");
		exit(0);
	};

}

/* Opens a shared memory (Readonly) and created it if necessary */
void* openShM(const char *name, uint32_t size)
{
	int fd;
	void *shm;
	fd = shm_open(name, O_RDONLY | O_CREAT, 777); //TODO change 777 back to 700
	if (fd == -1)
	{
		perror("SHM Open failed");
		return (NULL);
	}
	ftruncate(fd, size);
	shm = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
	if (MAP_FAILED == shm)
	{
		perror("SHM Map failed");
		shm = NULL;
		shm_unlink(name);
	}
	return shm;
}

/**
 * @brief writes data of shared memory into InfluxDB database
 *
 * @param[in] argc
 * @param[in] argv	-o            Reads main output variables from control
 *					-i            Reads main input variables to control
 *					-a            Reads additional output variables from control
 *					-t [value]    Specifies update-period in milliseconds. Default 10 seconds.
 *					-h            Prints this help message and exits
 * @return EXIT_SUCCSESS
 * @todo make ip, port, dbname, usr and pwd changeable
 */
int main(int argc, char *argv[])
{

	std::string ip = "127.0.0.1";
	int port = 8086;
	std::string dbname = "tsn_demo";
	std::string user = "admin";
	std::string pwd = "pass";

	struct demoreader_t reader;
	reader.flagaddout = false;
	reader.flagmainout = false;
	reader.flagmainin = false;
	reader.period = 10000000;       // 10 seconds
	struct timespec now;

	evalCLI(argc, argv, &reader);
	influxdb_cpp::server_info si = initDB(ip, port, dbname, user, pwd);

	//register signal handlers
	signal(SIGTERM, sigfunc);
	signal(SIGINT, sigfunc);

	// open and setup shm mapping
	std::cout << "setting up shared memory" << std::endl;
	if (reader.flagmainout)
	{
		reader.mainout = (struct mk_mainoutput*) openShM(MK_MAINOUTKEY,
				sizeof(struct mk_mainoutput));
		if (NULL == reader.mainout)
			reader.flagmainout = false;
	}
	if (reader.flagmainin)
	{
		reader.mainin = (struct mk_maininput*) openShM(MK_MAININKEY,
				sizeof(struct mk_maininput));
		if (NULL == reader.mainin)
			reader.flagmainin = false;
	}
	if (reader.flagaddout)
	{
		reader.addout = (struct mk_additionaloutput*) openShM(MK_ADDAOUTKEY,
				sizeof(struct mk_additionaloutput));
		if (NULL == reader.addout)
			reader.flagaddout = false;
	}

	// mainloop
	std::cout << "writing into database " << dbname << " every "
			<< reader.period / 1000000 << " s" << std::endl;
	while (run)
	{
		int r = clock_gettime( CLOCK_TAI, &now);
		if (reader.flagmainout)
		{
			influxdb_cpp::builder().meas("Main Output Variables")

			.field("X-Velocity Setpoint", reader.mainout->xvel_set).field(
					"Y-Velocity Setpoint", reader.mainout->yvel_set).field(
					"Z-Velocity Setpoint", reader.mainout->zvel_set).field(
					"Spindlespeed Setpoint", reader.mainout->spindlespeed)

			.field("X-Axis enabled", reader.mainout->xenable).field(
					"Y-Axis enabled", reader.mainout->yenable).field(
					"Z-Axis enabled", reader.mainout->zenable).field(
					"Spindle enabled", reader.mainout->spindleenable)

			.field("Spindlebrake", reader.mainout->spindlebrake).field(
					"Machine on", reader.mainout->machinestatus).field(
					"Emergency Stop activated", reader.mainout->estopstatus)

			.timestamp(now.tv_sec * 1e9 + now.tv_nsec).post_http(si);

		}
		if (reader.flagaddout)
		{
			influxdb_cpp::builder().meas("Additional Output Variables")

			.field("X-Position Setpoint", reader.addout->xpos_set).field(
					"Y-Position Setpoint", reader.addout->ypos_set).field(
					"Z-Position Setpoint", reader.addout->zpos_set).field(
					"Feedrate planned", reader.addout->feedrate)

			.field("X-Axis at home", reader.addout->xhome).field(
					"Y-Axis at home", reader.addout->yhome).field(
					"Z-Axis at home", reader.addout->zhome).field(
					"Feedrate override", reader.addout->feedoverride)

			.field("X-Axis at neg Endstop", reader.addout->xhardneg).field(
					"Y-Axis at neg Endstop", reader.addout->yhardneg).field(
					"Z-Axis at neg Endstop", reader.addout->zhardneg)

			.field("X-Axis at pos Endstop", reader.addout->xhardpos).field(
					"Y-Axis at pos Endstop", reader.addout->yhardpos).field(
					"Z-Axis at pos Endstop", reader.addout->zhardpos)

			.field("Current Line Number", reader.addout->lineno).field("Uptime",
					reader.addout->uptime)

			.field("Tool Number", (short int) reader.addout->tool).field("Mode",
					reader.addout->mode)

			.timestamp(now.tv_sec * 1e9 + now.tv_nsec).post_http(si);
		}
		if (reader.flagmainin)
		{

			influxdb_cpp::builder().meas("Main Input Variables")

			.field("X-Position Current", reader.mainin->xpos_cur).field(
					"Y-Position Current", reader.mainin->ypos_cur).field(
					"Z-Position Current", reader.mainin->zpos_cur)

			.field("X-Axis faulty", reader.mainin->xfault).field(
					"Y-Axis faulty", reader.mainin->yfault).field(
					"Z-Axis faulty", reader.mainin->zfault)

			.timestamp(now.tv_sec * 1e9 + now.tv_nsec).post_http(si);
		}

		std::cout << "wrote data for time " << now.tv_sec * 1e9 +now.tv_nsec << std::endl;
		usleep(reader.period);
	}

	return EXIT_SUCCESS;

}
