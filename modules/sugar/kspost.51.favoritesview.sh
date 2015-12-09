# Copyright (C) 2009 One Laptop Per Child
# Licensed under the terms of the GNU GPL v2 or later; see COPYING for details.

. $OOB__shlib

show=$(read_config sugar show_activities)
hide=$(read_config sugar hide_activities)

if [[ -n "$hide" ]]; then
	oIFS=$IFS
	IFS=$'\n\t, '
	for activity in $hide; do
		echo "echo $activity >> /usr/share/sugar/data/activities.hidden"
	done
	IFS=$oIFS
fi

if [[ -n "$show" ]]; then
	oIFS=$IFS
	IFS=$'\n\t, '
	for activity in $show; do
		echo "sed -i -e '/^$activity$/d' /usr/share/sugar/data/activities.hidden"
	done
	IFS=$oIFS
fi
