cat <<EOF >> /home/olpc/.xsession
if [ ! -e .lang-configured ]; then
    echo 'LANGUAGE="ht_HT.utf8:fr_FR.utf8"' >> .i18n
    touch .lang-configured
fi
EOF
