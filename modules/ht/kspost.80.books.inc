# changed schoolserver link
# redirect home page immediately if network is available
# if redirect fails, still show changed local schoolserver link
cat > /home/olpc/.library_pages/index.html <<EOF
<html><head>
<meta http-equiv="REFRESH" content="0; url=http://schoolserver/books/latest">
</head><body></body></html>
EOF
