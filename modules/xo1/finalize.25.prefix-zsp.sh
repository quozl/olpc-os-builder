# Copyright (C) 2009 One Laptop Per Child
# Licensed under the terms of the GNU GPL v2 or later; see COPYING for details.

. $OOB__shlib
make_zd=$(read_config sd_card_image make_zd)
osname=$(image_name)
prefix=modules/xo1/finalize.25.prefix-zsp.fth

function make_zd() {
	local output_name=$osname.zd
	local output=$outputdir/$output_name

	if [[ "$make_zd" == 1 ]]; then
	        echo $(pwd)
		echo "Prefix XO-1 fs-update for $output_name..."
		(
			cat $prefix
			echo "\ above from olpc-os-builder:$prefix"
			echo "\ below from $output.zsp, minus header"
			awk '/^data: /{x=1;} {if(x)print $0;}' $output.zsp
		) > $output.zsp.new
		mv $output.zsp.new $output.zsp
	fi
}

make_zd
