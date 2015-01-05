# enlarge cursor #12848
cat >/usr/share/glib-2.0/schemas/mate.oob.gschema.override <<EOF
[org.mate.peripherals-mouse]
cursor-size=48
EOF
/usr/bin/glib-compile-schemas /usr/share/glib-2.0/schemas
