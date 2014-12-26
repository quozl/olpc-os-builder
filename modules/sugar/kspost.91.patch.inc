# FIXME: waiting for a sugar release
# after sugar-0.103.1-1.fc22 and
# after merge of pull request
# https://github.com/sugarlabs/sugar/pull/458

# configure.ac: fix version for 0.103.1
sed -i -e "s/version = '0.102.0'/version = '0.103.1'/g" \
  /usr/lib/python2.7/site-packages/jarabe/config.py
