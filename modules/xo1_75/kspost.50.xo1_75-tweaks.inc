# enable serial console on ttyS2
ln -sf /lib/systemd/system/serial-getty@.service \
	/etc/systemd/system/getty.target.wants/serial-getty@ttyS2.service

# enable serial console for runin too
mkdir -p /etc/systemd/system/runin.target.wants
ln -sf /lib/systemd/system/serial-getty@.service \
	/etc/systemd/system/runin.target.wants/serial-getty@ttyS2.service

# hide the audio icon while it does not work #12867
(cd /usr/share/sugar/extensions/deviceicon && mv audio.py audio.py.orig)

cat >> /usr/local/bin/olpc-12867-reproduce <<EOF
#!/bin/bash
(cd /usr/share/sugar/extensions/deviceicon && sudo mv audio.py.orig audio.py)
sudo service olpc-dm restart
EOF
chmod +x /usr/local/bin/olpc-12867-reproduce
