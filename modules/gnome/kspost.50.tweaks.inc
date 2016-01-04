# take back control of keyboard, to fix language switch key
cat >/usr/share/glib-2.0/schemas/gnome.oob.gschema.override <<EOF
[org.gnome.settings-daemon.plugins.keyboard]
active=false
EOF
/usr/bin/glib-compile-schemas /usr/share/glib-2.0/schemas
