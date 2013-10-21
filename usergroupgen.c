/*
 * Contact: Carsten Munk <carsten.munk@jollamobile.com>
 *
 * Copyright (c) 2013, Jolla Ltd.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * * Neither the name of the <organization> nor the
 * names of its contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "android_filesystem_config.h"
#include <stdio.h>
#include <assert.h>

int do_group(int add)
{
	int i;
	for (i = 0; i < android_id_count; i++)
		if (android_ids[i].aid != 0)
			if (add == 1)
				printf("groupadd -g %i %s\n", android_ids[i].aid,
					   android_ids[i].name);
			else
				printf("groupdel %s\n", android_ids[i].name);
}

int main(int argc, char *argv[])
{
	int i;
	int add = 1;
	if (argc == 2 && strcmp("remove", argv[1]) == 0)
		add = 0;
	printf("#!/bin/sh\n");
	/* Add groups before users */
	if (add == 1)
		do_group(add);

	for (i = 0; i < android_id_count; i++)
		if (android_ids[i].aid != 0)
			if (add == 1)
				printf("useradd -M -N -s /sbin/nologin -d / -u %i -g %i %s\n",
					   android_ids[i].aid, android_ids[i].aid,
					   android_ids[i].name);
			else
				printf("userdel -f %s\n", android_ids[i].name);
	
	/* Remove groups after users */
	if (add == 0)
		do_group(add);

	return 0;
}
