# enable serial console for runin
mkdir -p /etc/systemd/system/runin.target.wants
ln -sf /lib/systemd/system/serial-getty@.service \
	/etc/systemd/system/runin.target.wants/serial-getty@ttyS2.service
