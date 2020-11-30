# SPDX-License-Identifier: (MIT)
#
# Copyright (c) 2020 Institute for Control Engineering of Machine Tools and Manufacturing Units, University of Stuttgart
# Author: Philipp Neher <philipp.neher@isw.uni-stuttgart.de>
#         Nico Brandt

INC = -I. -I.. -I../influxdb-cpp/
CC=g++
CFLAGS=$(INC) -g

ODIR=obj
LDIR=../lib

LIBS=-lrt -pthread

_OBJ = 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c
	@mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

all: write2influxdb

write2influxdb: write2influxdb.cpp
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ write2influxdb