# configure harvest client
gconftool-2 --direct \
    --config-source xml:readwrite:/etc/gconf/gconf.xml.defaults \
    --set /desktop/sugar/collaboration/harvest_hostname \
    "https://50.17.210.12:8120" --type string
gconftool-2  --direct \
    --config-source xml:readwrite:/etc/gconf/gconf.xml.defaults \
    --set /desktop/sugar/collaboration/harvest_api_key \
    harvest-haiti --type string
