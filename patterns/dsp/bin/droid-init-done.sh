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

#
# This informs systemd that droid-hal-init has been started
# This service should be enabled in one of the /init*.rc files like this:

# service droid_init_done /bin/sh /usr/bin/droid/droid-init-done.sh
#    class xxx (xxx = some late starting class)
#    oneshot

export LD_LIBRARY_PATH=/lib:/usr/lib
export PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin:/root/bin
export NOTIFY_SOCKET="@/org/freedesktop/systemd1/notify"
DROID_PID=$(pgrep droid-hal-init)
systemd-notify --pid=$DROID_PID --ready
# Systemd has a bug and can't handle the situation that notifying daemon (this one)
# does exit before systemd has fully handled the notify message.
# Thus we need to stay here and make sure systemd has handled our notify message
n=0
while [ $n -lt 3 ]; do
    sleep 1
    droid_status=`systemctl is-active droid-hal-init.service`
    if [ "$droid_status" == "active" ]; then
        break
    fi
    echo "info systemd again..."
    systemd-notify --pid=$DROID_PID --ready
    let n=$n+1
done

if [ "$droid_status" == "active" ]; then
    exit 0
else
    echo "Couldn't deliver notify message to systemd"
    exit 1
fi
