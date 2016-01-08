/usr/sbin/chkconfig sshd on
mkdir /root/.ssh
chmod 0700 /root/.ssh
cat <<EOF > /root/.ssh/authorized_keys
ssh-rsa AAAAB3NzaC1yc2EAAAABJQAAAQEAkNaMyuJ5reW3sDhdd5IMxmGKfuRCURBhL09KdjVj/I/Ut2lYCrcfOsKEC+5rsfnFu4UAt0dqcz739Rr+dGaqIkuCkGRNDqe/ZVTWHMXqC9tcvn3kznky1nsSgPE8R8cI3wEKpvUGTeizXB8Pn6Rg8/yx6jbObskefZWsCz6tTQxEZJZyof8NeksmOE2XtjSYBBhARHBLaW5wwGE9vHqVJrR+MzmZRPK5yVnmwf59mGbKJ8YgFCrUqhK/Ao/OAGezotAfMFIu3PhsFutxpY3/g+UnqLdKWX6LSTdtEbhvA+gkBCjHHNEBiN+u2UxBMP0vMBZ9X9/ONk6SDFaHdrji1w== sora
EOF
chmod 0600 /root/.ssh/authorized_keys
