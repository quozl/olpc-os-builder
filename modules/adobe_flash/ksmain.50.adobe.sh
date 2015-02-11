# Copyright (C) 2012 One Laptop Per Child
# Licensed under the terms of the GNU GPL v2 or later; see COPYING for details.

. $OOB__shlib

# Only include the repo when a local plugin hasn't been provided

path=$(read_config adobe_flash plugin_path)
if [ -n "$path" ]; then
    [ ! -e "$path" ] && echo "adobe_flash.plugin_path is invalid" >&2 && exit 1
    exit 0
fi

echo "repo --name=adobe --baseurl=http://linuxdownload.adobe.com/linux/i386/"
