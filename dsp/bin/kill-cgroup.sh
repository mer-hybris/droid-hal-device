#!/bin/sh
# Contact: Pekka Lundstrom  <pekka.lundstrom@jollamobile.com>
#
# Copyright (c) 2013, Jolla Ltd.
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the <organization> nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


# Kill all processes that are in this same cgroup ($1) 
[ -z "$1" ] && echo "Need cgroup path" && exit 1
CGROUP=$1
[ ! -f /sys/fs/cgroup/systemd/$CGROUP/cgroup.procs ] && echo "No such cgroup: $1" && exit 1

get_pids() {
    # Get list of running pids in this cgroup
    # return list $PIDS and $NUM_PIDS
    PIDS=$(cat /sys/fs/cgroup/systemd/$CGROUP/cgroup.procs)
    NUM_PIDS=$(echo $PIDS | wc -w)
}

kill_pids() {
    # Kill pids using signal $1
    for pid in $PIDS; do
         [ -d /proc/$pid ] && kill -$1 $pid
    done
}

# ============== main() ===============

get_pids
PREV_NUM_PIDS=$NUM_PIDS
kill_pids TERM
sleep 1
WAIT=1
get_pids
MAX_WAIT=5
while [ $NUM_PIDS -gt 0 -a $WAIT -lt $MAX_WAIT ]; do
    if [ $NUM_PIDS -lt $PREV_NUM_PIDS ]; then
        # Number of running processes is getting smaller
        # Wait a little bit more
        sleep 1
     else
        # Number of pids is not gettting smaller
        break
    fi
    let WAIT=$WAIT+1
    PREV_NUM_PIDS=$NUM_PIDS
    get_pids
done
# If anyone left, hard kill them all
[ $NUM_PIDS -gt 0 ] && kill_pids KILL
exit 0

