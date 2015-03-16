patch -f /usr/share/sugar/extensions/cpsection/aboutcomputer/model.py <<EOF || true
From 2e54a87c96eae767d3b41953f9bc204c4a622d96 Mon Sep 17 00:00:00 2001
From: James Cameron <quozl@laptop.org>
Date: Mon, 9 Feb 2015 14:34:18 +1100
Subject: [PATCH] aboutcomputer: redesign for OLPC only

- rely on wireless on eth0, avoid NMClient and NetworkManager delays,
  see OLPC #12856,

- show model, using MN tag,

- show SKU,

- show EC firmware,

- show wireless driver, card, and maximum capability,

- simplify device tree access,
---
 model.py | 82 +++++++++++++++++++++++++++-------------------------------------
 1 file changed, 35 insertions(+), 47 deletions(-)

diff --git a/model.py b/model.py
index 2dd79d9..8aaf0bf 100644
--- a/model.py
+++ b/model.py
@@ -23,8 +23,6 @@ import errno
 import time
 
 from gi.repository import Gio
-from gi.repository import NMClient
-from gi.repository import NetworkManager
 
 from jarabe import config
 
@@ -32,8 +30,6 @@ from jarabe import config
 _OFW_TREE = '/ofw'
 _PROC_TREE = '/proc/device-tree'
 _DMI_DIRECTORY = '/sys/class/dmi/id'
-_SN = 'serial-number'
-_MODEL = 'openprom/model'
 
 _logger = logging.getLogger('ControlPanel - AboutComputer')
 _not_available = _('Not available')
@@ -50,11 +46,7 @@ def print_aboutcomputer():
 
 
 def get_serial_number():
-    serial_no = None
-    if os.path.exists(os.path.join(_OFW_TREE, _SN)):
-        serial_no = _read_file(os.path.join(_OFW_TREE, _SN))
-    elif os.path.exists(os.path.join(_PROC_TREE, _SN)):
-        serial_no = _read_file(os.path.join(_PROC_TREE, _SN))
+    serial_no = _read_device_tree('serial-number')
     if serial_no is None:
         serial_no = _not_available
     return serial_no
@@ -93,35 +85,23 @@ def print_build_number():
     print get_build_number()
 
 
-def _parse_firmware_number(firmware_no):
-    if firmware_no is None:
-        firmware_no = _not_available
-    else:
-        # try to extract Open Firmware version from OLPC style version
-        # string, e.g. "CL2   Q4B11  Q4B"
+def get_firmware_number():
+    firmware_no = _read_device_tree('openprom/model')
+    if firmware_no is not None:
         if firmware_no.startswith('CL'):
-            firmware_no = firmware_no[6:13]
-    return firmware_no
+            firmware_no = firmware_no[6:13].strip()
+        firmware_no = firmware_no + ' with EC ' + _read_device_tree('ec-name')
+        return firmware_no
 
-
-def get_firmware_number():
-    firmware_no = None
-    if os.path.exists(os.path.join(_OFW_TREE, _MODEL)):
-        firmware_no = _read_file(os.path.join(_OFW_TREE, _MODEL))
-        firmware_no = _parse_firmware_number(firmware_no)
-    elif os.path.exists(os.path.join(_PROC_TREE, _MODEL)):
-        firmware_no = _read_file(os.path.join(_PROC_TREE, _MODEL))
-        firmware_no = _parse_firmware_number(firmware_no)
-    elif os.path.exists(os.path.join(_DMI_DIRECTORY, 'bios_version')):
-        firmware_no = _read_file(os.path.join(_DMI_DIRECTORY, 'bios_version'))
-        if firmware_no is None:
-            firmware_no = _not_available
+    firmware_no = _read_file(os.path.join(_DMI_DIRECTORY, 'bios_version'))
+    if firmware_no is None:
+        firmware_no = _not_available
     return firmware_no
 
 
 def get_hardware_model():
-    settings = Gio.Settings('org.sugarlabs.extensions.aboutcomputer')
-    return settings.get_string('hardware-model')
+    return '%s (SKU%s)' % (_read_device_tree('mfg-data/MN'),
+                           _read_device_tree('mfg-data/sk'))
 
 
 def get_secondary_licenses():
@@ -144,18 +124,7 @@ def print_firmware_number():
 
 
 def _get_wireless_interfaces():
-    try:
-        network_manager = NMClient.Client()
-    except:
-        return ['wlan0', 'eth0']
-
-    interfaces = []
-    for device in network_manager.get_devices():
-        if device.get_device_type() is not NetworkManager.DeviceType.WIFI:
-            continue
-        interfaces.append(device.get_iface())
-
-    return interfaces
+    return ['eth0']
 
 
 def get_wireless_firmware():
@@ -172,14 +141,23 @@ def get_wireless_firmware():
             continue
 
         try:
-            version = ([line for line in output
-                        if line.startswith('firmware')][0].split()[1])
+            for line in output:
+                if line.startswith('firmware'):
+                    version = line.split()[1]
+                if line.startswith('driver'):
+                    driver = line.split()[1]
         except IndexError:
             _logger.exception('Error parsing ethtool output for %r',
                               interface)
             continue
 
-        firmware_info[interface] = version
+        card = 'unknown'
+        if driver == 'mwifiex':
+            card = 'mv8787, IEEE 802.11n 5GHz'
+        if driver == 'libertas':
+            card = 'mv8686, IEEE 802.11g 2.4GHz'
+
+        firmware_info[interface] = '%s (%s, %s)' % (version, driver, card)
 
     if not firmware_info:
         return _not_available
@@ -211,6 +189,16 @@ def _read_file(path):
         return None
 
 
+def _read_device_tree(path):
+    value = _read_file(os.path.join(_PROC_TREE, path))
+    if value:
+        return value.strip('\x00')
+    value = _read_file(os.path.join(_OFW_TREE, path))
+    if value:
+        return value.strip('\x00')
+    return value
+
+
 def get_license():
     license_file = os.path.join(config.data_path, 'GPLv2')
     lang = os.environ['LANG']
-- 
1.8.3.2


EOF
patch -f /usr/lib/python2.7/site-packages/jarabe/model/shell.py <<EOF || true
From c26fa216b99c25b735169744aac3830877ed83f9 Mon Sep 17 00:00:00 2001
From: James Cameron <quozl@laptop.org>
Date: Mon, 16 Mar 2015 18:35:04 +1100
Subject: [PATCH] prevent duplicate launch timers

"failed to start" will occur if an activity is started exactly 90
seconds before any previous start.  This is easy to reproduce if an
activity is repeatedly started and stopped.

This was caused by the previous launch timers.  There was no limit to
the number of launch timers that may be pending.

Delete any existing launch timer before starting a new one.  Also
removed stale FIXME for status checking; this is done in the toolkit.
(in _child_watch_cb method of activityfactory.py)

Fixes #2974.

Tested-by: James Cameron <quozl@laptop.org>
Signed-off-by: James Cameron <quozl@laptop.org>
---
 src/jarabe/model/shell.py | 12 +++++++++---
 1 file changed, 9 insertions(+), 3 deletions(-)

diff --git a/src/jarabe/model/shell.py b/src/jarabe/model/shell.py
index d46585f..5f4f352 100644
--- a/src/jarabe/model/shell.py
+++ b/src/jarabe/model/shell.py
@@ -429,6 +429,8 @@ class ShellModel(GObject.GObject):
         self._maximum_open_activities = settings.get_int(
             'maximum-number-of-open-activities')
 
+        self._launch_timers = {}
+
     def get_launcher(self, activity_id):
         return self._launchers.get(str(activity_id))
 
@@ -732,10 +734,13 @@ class ShellModel(GObject.GObject):
 
         self.emit('launch-started', home_activity)
 
-        # FIXME: better learn about finishing processes by receiving a signal.
-        # Now just check whether an activity has a window after ~90sec
-        GObject.timeout_add_seconds(90, self._check_activity_launched,
+        if activity_id in self._launch_timers:
+            GObject.source_remove(self._launch_timers[activity_id])
+            del self._launch_timers[activity_id]
+
+        timer = GObject.timeout_add_seconds(90, self._check_activity_launched,
                                     activity_id)
+        self._launch_timers[activity_id] = timer
 
     def notify_launch_failed(self, activity_id):
         home_activity = self.get_activity_by_id(activity_id)
@@ -752,6 +757,7 @@ class ShellModel(GObject.GObject):
                           activity_id)
 
     def _check_activity_launched(self, activity_id):
+        del self._launch_timers[activity_id]
         home_activity = self.get_activity_by_id(activity_id)
 
         if not home_activity:
-- 
1.8.3.2

EOF
