cat <<EOF >> /etc/rc.d/rc.local
#!/bin/sh
if [ \$(date +%Y) -lt 2016 ]; then
    date --utc --set="2016-01-01 01:01:01"
    hwclock --systohc
fi
EOF
chmod +x /etc/rc.d/rc.local
