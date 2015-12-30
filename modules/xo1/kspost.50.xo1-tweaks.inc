# do not show the default ad-hoc networks
# reduce animation rate during activity start
cat >/usr/share/glib-2.0/schemas/sugar.oob.xo1.gschema.override <<EOF
[org.sugarlabs.network]
adhoc=false

[org.sugarlabs.desktop]
launcher-interval=500
EOF
/usr/bin/glib-compile-schemas /usr/share/glib-2.0/schemas

# enable serial console on ttyS0
ln -sf /lib/systemd/system/serial-getty@.service \
	/etc/systemd/system/getty.target.wants/serial-getty@ttyS0.service

# allocate a swap file
dd if=/dev/zero of=/var/swap bs=1024k count=64
