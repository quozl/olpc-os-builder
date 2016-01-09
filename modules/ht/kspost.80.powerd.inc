# set default brightness half way
cat <<EOF >> /etc/powerd/powerd.conf
config_INITIAL_BRIGHT_LEVEL="7"
config_IDLE_DIM_LEVEL="6"
EOF
