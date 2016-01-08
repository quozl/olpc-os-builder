cat <<EOF >>/etc/rc.d/rc.local
#!/bin/sh
if [ ! -e /.ht-configured ]; then
    if [ -e /dev/sda1 ]; then
        if mount /dev/sda1 /mnt; then
            cd /etc/NetworkManager/system-connections
            for file in *; do
                case "\$file" in
                    OLPC\ Mesh*|OLPC\ XS*|Sugar\ Modem*)
                        ;;
                    *)
                        if [ ! -e "/mnt/\$file" ]; then
                            rm -f \$file
                        fi
                        ;;
                esac
            done
            umount /mnt
        fi
    fi
    touch /.ht-configured
fi
EOF
chmod +x /etc/rc.d/rc.local
