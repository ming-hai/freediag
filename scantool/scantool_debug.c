/*
 *	freediag - Vehicle Diagnostic Utility
 *
 *
 * Copyright (C) 2001 Richard Almeida & Ibex Ltd (rpa@ibex.co.uk)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *************************************************************************
 *
 *
 * Mostly ODBII Compliant Scan Tool (as defined in SAE J1978)
 *
 * CLI routines - debug subcommand
 */

#include "diag.h"
#include "diag_tty.h"
#include "diag_l1.h"
#include "diag_l2.h"
#include "diag_l3.h"

#include "scantool.h"
#include "scantool_cli.h"

CVSID("$Id$");

int diag_cmd_debug;

static int cmd_debug_dumpdata(int argc, char **argv);
static int cmd_debug_pids(int argc, char **argv);
static int cmd_debug_help(int argc, char **argv);
static int cmd_debug_show(int argc, char **argv);

static int cmd_debug_cli(int argc, char **argv);
static int cmd_debug_l0(int argc, char **argv);
static int cmd_debug_l1(int argc, char **argv);
static int cmd_debug_l2(int argc, char **argv);
static int cmd_debug_l3(int argc, char **argv);
static int cmd_debug_all(int argc, char **argv);
static int cmd_debug_l0test(int argc, char **argv);

const struct cmd_tbl_entry debug_cmd_table[] =
{
	{ "help", "help [command]", "Gives help for a command",
		cmd_debug_help, 0, NULL},
	{ "?", "? [command]", "Gives help for a command",
		cmd_debug_help, FLAG_HIDDEN, NULL},

	{ "dumpdata", "dumpdata", "Show Mode1 Pid1/2 responses",
		cmd_debug_dumpdata, 0, NULL},

	{ "pids", "pids", "Shows PIDs supported by ECU",
		cmd_debug_pids, 0, NULL},

	{ "show", "show", "Shows current debug levels",
		cmd_debug_show, 0, NULL},

	{ "l0", "l0 [val]", "Show/set Layer0 debug level",
		cmd_debug_l0, 0, NULL},
	{ "l1", "l1 [val]", "Show/set Layer1 debug level",
		cmd_debug_l1, 0, NULL},
	{ "l2", "l2 [val]", "Show/set Layer2 debug level",
		cmd_debug_l2, 0, NULL},
	{ "l3", "l3 [val]", "Show/set Layer3 debug level",
		cmd_debug_l3, 0, NULL},
	{ "cli", "cli [val]", "Show/set CLI debug level",
		cmd_debug_cli, 0, NULL},
	{ "all", "all [val]", "Show/set All layer debug level",
		cmd_debug_all, 0, NULL},
	{ "l0test", "l0test [testnum]", "Dumb interface tests. Disconnect from vehicle first !",
		cmd_debug_l0test, 0, NULL},
	{ "up", "up", "Return to previous menu level",
		cmd_up, 0, NULL},
	{ "quit","quit", "Return to previous menu level",
		cmd_up, FLAG_HIDDEN, NULL},
	{ "exit", "exit", "Exit program",
		cmd_exit, 0, NULL},

	{ NULL, NULL, NULL, NULL, 0, NULL}
};

static int
cmd_debug_help(int argc, char **argv)
{
	if (argc<2) {
		printf("Debugging flags are set per level according to the values set in diag.h\n");
		printf("Setting [val] to -1 will enable all debug messages for that level.\n");
	}
	return help_common(argc, argv, debug_cmd_table);
}


static void
print_resp_info(UNUSED(int mode), response_t *data)
{

	int i;
	for (i=0; i<256; i++)
	{
		if (data->type != TYPE_UNTESTED)
		{
			if (data->type == TYPE_GOOD)
			{
				int j;
				printf("0x%02X: ", i );
				for (j=0; j<data->len; j++)
					printf("%02X ", data->data[j]);
				printf("\n");
			}
			else
				printf("0x%02X: Failed 0x%X\n",
					i, data->data[1]);
		}
		data++;
	}
}


static int
cmd_debug_dumpdata(UNUSED(int argc), UNUSED(char **argv))
{
	ecu_data_t *ep;
	int i;

	printf("Current Data\n");
	for (i=0, ep=ecu_info; i<MAX_ECU; i++,ep++)
	{
		if (ep->valid)
		{
			printf("ECU %d:\n", ep->ecu_addr & 0xff);
			print_resp_info(1, ep->mode1_data);
		}
	}

	printf("Freezeframe Data\n");
	for (i=0,ep=ecu_info; i<MAX_ECU; i++,ep++)
	{
		if (ep->valid)
		{
			printf("ECU %d:\n", ep->ecu_addr & 0xff);
			print_resp_info(2, ep->mode2_data);
		}
	}

	return CMD_OK;
}

static int
cmd_debug_common( const char *txt, int *val, int argc, char **argv)
{
	int r;

	if (argc == 1)
	{
		printf("%s debug is 0x%X\n", txt, *val);
	}
	else
	{
		r = htoi(argv[1]);
		*val = r;
	}
	return CMD_OK;
}

static int
cmd_debug_l0(int argc, char **argv)
{
	return cmd_debug_common("L0", &diag_l0_debug, argc, argv);
}
static int
cmd_debug_l1(int argc, char **argv)
{
	return cmd_debug_common("L1", &diag_l1_debug, argc, argv);
}
static int
cmd_debug_l2(int argc, char **argv)
{
	return cmd_debug_common("L2", &diag_l2_debug, argc, argv);
}
static int
cmd_debug_l3(int argc, char **argv)
{
	return cmd_debug_common("L3", &diag_l3_debug, argc, argv);
}
static int
cmd_debug_cli(int argc, char **argv)
{
	return cmd_debug_common("CLI", &diag_cmd_debug, argc, argv);
	//for now, value > 0x80 will enable all debugging info.
}

static int
cmd_debug_all(int argc, char **argv)
{
	int val;

	if (argc > 0) {
		val = htoi(argv[1]);
		diag_l0_debug = val;
		diag_l1_debug = val;
		diag_l2_debug = val;
		diag_l3_debug = val;
		diag_cmd_debug = val;
	}
	return cmd_debug_show(1, NULL);

	return CMD_OK;
}


static int
cmd_debug_show(UNUSED(int argc), UNUSED(char **argv))
{
/*	int layer, val; */

	printf("Debug values: L0 0x%X, L1 0x%X, L2 0x%X L3 0x%X CLI 0x%X\n",
		diag_l0_debug, diag_l1_debug, diag_l2_debug, diag_l3_debug,
		diag_cmd_debug);
	return CMD_OK;
}

static void
print_pidinfo(int mode, uint8_t *pid_data)
{
	int i,j,p;

	j = 0; p = 0;
	printf(" Mode %d:\n	", mode);
	for (i=0; i<=0x60; i++)
	{
		if (pid_data[i]) {
			printf("0x%X ", i);
			j++; p++;
		}
		if (j == 10)
		{
			j = 0;
			printf("\n	");
		}
	}
	if ((p == 0) || (j != 0))
		printf("\n");
}


static int cmd_debug_pids(UNUSED(int argc), UNUSED(char **argv))
{
	ecu_data_t *ep;
	int i;

	if (global_state < STATE_SCANDONE)
	{
		printf("SCAN has not been done, please do a scan\n");
		return CMD_OK;
	}

	for (i=0,ep=ecu_info; i<MAX_ECU; i++,ep++)
	{
		if (ep->valid)
		{
			printf("ECU %d address 0x%X: Supported PIDs:\n",
				i, ep->ecu_addr & 0xff);
			print_pidinfo(1, ep->pids);
			print_pidinfo(2, ep->mode2_info);
			print_pidinfo(5, ep->mode5_info);
			print_pidinfo(6, ep->mode6_info);
			print_pidinfo(8, ep->mode8_info);
			print_pidinfo(9, ep->mode9_info);
		}
	}
	printf("\n");

	return CMD_OK;
}

//cmd_debug_l0test : run a variety of low-level
//tests, for dumb interfaces. Do not use while connected
//to a vehicle: this sends garbage data on the K-line which
//could interfere with ECUs, although very unlikely.
//XXX unfinished; I'm implementing this as another L0 driver
//(diag_l0_dumbtest.c).
static int cmd_debug_l0test(int argc, char **argv) {
#define MAX_L0TEST 6
	unsigned int testnum=0;
	if ((argc <= 1) || (strcmp(argv[1], "?") == 0) || (sscanf(argv[1],"%u", &testnum) != 1)) {
		printf("usage: %s [testnum], where testnum is a number between 1 and %d.\n", argv[0], MAX_L0TEST);
		printf("you must have done \"set interface dumbt [port]\" and \"set dumbopts\" before proceding.\n");

		printf("Available tests:\n"
				"\t1 : slow pulse TXD (K) with diag_tty_break.\n"
				"\t2 : fast pulse TXD (K) : send 0x55 @ 10400bps, 5ms interbyte (P4)\n"
				"\t3 : slow pulse RTS.\n"
				"\t4 : slow pulse DTR.\n"
				"\t5 : fast pulse TXD (K) with diag_tty_break.\n"
				"\t6 : fast pulse TXD (K) with diag_tty_fastbreak.\n");
		return CMD_OK;
	}
	if ((testnum < 1) || (testnum > MAX_L0TEST)) {
		printf("Invalid test.\n");
		return CMD_FAILED;
	}

	if (diag_init())
		return CMD_FAILED;

	printf("Trying test %u on %s...\n", testnum, set_subinterface);

	// I think the easiest way to pass on "testnum" on to diag_l0_dumbtest.c is
	// to pretend testnum is an L1protocol. Then we can use diag_l2_open to start the
	// test.
	diag_l2_open("DUMBT", set_subinterface, (int) testnum);
	return CMD_OK;


}

