# 
# These are selected changes to Sugar since v0.104.1 for which an RPM
# is not yet available, from tag v0.104.1, to hash
# 91785c8c8ed6d7061482a34c15b8e46f9fd586f0
# 
# Selected changes are:
#     suppress __set_current_activity_cb message
#     prevent duplicate launch timers
#     fix language control panel on XO es_UY keyboards
#     Update copyright date to 2015 in my settings
#     v0.105.0
#     Avoid control panel performance regression - Fixes OLPC #12856
#     Show information about the wifi card and driver if available
#     Add more hardware information for XO devices
#     Update card description to include XO-1 harware
#     Avoid blank space below build
#     Detect XO-1 wireless hardware
#     Avoid split of wireless firmware words
#     Add accelerator for control panel close
#
# Affected packages are sugar and sugar-cp-language.
#
# Risk: if any of these patches are pulled before Sugar 0.106 release
# then a regression may be observed on upgrade.
#
patch -f /usr/share/sugar/extensions/cpsection/aboutcomputer/model.py <<EOF || true
diff -ru sugar-v0.104.1/extensions/cpsection/aboutcomputer/model.py sugar-91785c8/extensions/cpsection/aboutcomputer/model.py
--- sugar-v0.104.1/extensions/cpsection/aboutcomputer/model.py	2015-05-09 08:18:33.286565617 +1000
+++ sugar-91785c8/extensions/cpsection/aboutcomputer/model.py	2015-05-08 18:24:32.408245670 +1000
@@ -23,17 +23,14 @@
 import time
 
 from gi.repository import Gio
-from gi.repository import NMClient
-from gi.repository import NetworkManager
 
 from jarabe import config
+from jarabe.model.network import get_wireless_interfaces
 
 
 _OFW_TREE = '/ofw'
 _PROC_TREE = '/proc/device-tree'
 _DMI_DIRECTORY = '/sys/class/dmi/id'
-_SN = 'serial-number'
-_MODEL = 'openprom/model'
 
 _logger = logging.getLogger('ControlPanel - AboutComputer')
 _not_available = _('Not available')
@@ -50,11 +47,7 @@
 
 
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
@@ -82,6 +75,7 @@
                 raise
         else:
             build_no, stderr_ = popen.communicate()
+            build_no = build_no.strip()
 
     if build_no is None or not build_no:
         build_no = _not_available
@@ -93,35 +87,33 @@
     print get_build_number()
 
 
-def _parse_firmware_number(firmware_no):
-    if firmware_no is None:
-        firmware_no = _not_available
-    else:
+def get_firmware_number():
+    firmware_no = _read_device_tree('openprom/model')
+    if firmware_no is not None:
         # try to extract Open Firmware version from OLPC style version
         # string, e.g. "CL2   Q4B11  Q4B"
         if firmware_no.startswith('CL'):
             firmware_no = firmware_no[6:13]
-    return firmware_no
-
+        ec_name = _read_device_tree('ec-name')
+        if ec_name:
+            firmware_no = '%(firmware)s with %(ec)s' % {
+                'firmware': firmware_no, 'ec': ec_name}
 
-def get_firmware_number():
-    firmware_no = None
-    if os.path.exists(os.path.join(_OFW_TREE, _MODEL)):
-        firmware_no = _read_file(os.path.join(_OFW_TREE, _MODEL))
-        firmware_no = _parse_firmware_number(firmware_no)
-    elif os.path.exists(os.path.join(_PROC_TREE, _MODEL)):
-        firmware_no = _read_file(os.path.join(_PROC_TREE, _MODEL))
-        firmware_no = _parse_firmware_number(firmware_no)
     elif os.path.exists(os.path.join(_DMI_DIRECTORY, 'bios_version')):
         firmware_no = _read_file(os.path.join(_DMI_DIRECTORY, 'bios_version'))
-        if firmware_no is None:
-            firmware_no = _not_available
+    if firmware_no is None:
+        firmware_no = _not_available
     return firmware_no
 
 
 def get_hardware_model():
     settings = Gio.Settings('org.sugarlabs.extensions.aboutcomputer')
-    return settings.get_string('hardware-model')
+    model = settings.get_string('hardware-model')
+    if not model:
+        model = _read_device_tree('mfg-data/MN')
+        sku = _read_device_tree('mfg-data/sk')
+        if sku:
+            model = '%s (SKU%s)' % (model, sku)
 
 
 def get_secondary_licenses():
@@ -143,26 +135,18 @@
     print get_firmware_number()
 
 
-def _get_wireless_interfaces():
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
-
-
 def get_wireless_firmware():
     environment = os.environ.copy()
     environment['PATH'] = '%s:/usr/sbin' % (environment['PATH'], )
     firmware_info = {}
-    for interface in _get_wireless_interfaces():
+
+    wireless_interfaces = get_wireless_interfaces()
+    if not wireless_interfaces:
+        _logger.warning('Cannot connect to NetworkManager, falling back to'
+                        ' static list of devices')
+        wireless_interfaces = ['wlan0', 'eth0']
+
+    for interface in get_wireless_interfaces():
         try:
             output = subprocess.Popen(['ethtool', '-i', interface],
                                       stdout=subprocess.PIPE,
@@ -172,14 +156,29 @@
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
+        card = None
+        if driver == 'mwifiex':
+            card = 'mv8787, IEEE 802.11n 5GHz'
+        if driver == 'libertas':
+            if version.startswith('5.'):
+                card = 'usb8388, IEEE 802.11g 2.4GHz'
+            else:
+                card = 'mv8686, IEEE 802.11g 2.4GHz'
+
+        if card:
+            firmware_info[interface] = '%s (%s, %s)' % (version, driver, card)
+        else:
+            firmware_info[interface] = '%s (%s)' % (version, driver)
 
     if not firmware_info:
         return _not_available
@@ -187,9 +186,9 @@
     if len(firmware_info) == 1:
         return firmware_info.values()[0]
 
-    return ', '.join(['%(interface)s: %(version)s' %
-                      {'interface': interface, 'version': version}
-                      for interface, version in firmware_info.items()])
+    return ', '.join(['%(interface)s: %(info)s' %
+                      {'interface': interface, 'info': info}
+                      for interface, info in firmware_info.items()])
 
 
 def print_wireless_firmware():
@@ -235,6 +234,16 @@
     return license_text
 
 
+def _read_device_tree(path):
+    value = _read_file(os.path.join(_PROC_TREE, path))
+    if value:
+        return value.strip('\\x00')
+    value = _read_file(os.path.join(_OFW_TREE, path))
+    if value:
+        return value.strip('\\x00')
+    return value
+
+
 def days_from_last_update():
 
     last_update_seconds = -1
EOF
patch -f /usr/share/sugar/extensions/cpsection/aboutcomputer/view.py <<EOF || true
diff -ru sugar-v0.104.1/extensions/cpsection/aboutcomputer/view.py sugar-91785c8/extensions/cpsection/aboutcomputer/view.py
--- sugar-v0.104.1/extensions/cpsection/aboutcomputer/view.py	2015-05-09 08:18:33.286565617 +1000
+++ sugar-91785c8/extensions/cpsection/aboutcomputer/view.py	2015-05-08 18:24:32.428245406 +1000
@@ -155,7 +155,7 @@
         vbox_copyright.set_border_width(style.DEFAULT_SPACING * 2)
         vbox_copyright.set_spacing(style.DEFAULT_SPACING)
 
-        copyright_text = '© 2006-2013 One Laptop per Child Association Inc,' \\
+        copyright_text = '© 2006-2015 One Laptop per Child Association Inc,' \\
                          ' Sugar Labs Inc, Red Hat Inc, Collabora Ltd and' \\
                          ' Contributors.'
         label_copyright = Gtk.Label(label=copyright_text)
EOF
# package sugar-cp-language
patch -f /usr/share/sugar/extensions/cpsection/language/model.py <<EOF || true
diff -ru sugar-v0.104.1/extensions/cpsection/language/model.py sugar-91785c8/extensions/cpsection/language/model.py
--- sugar-v0.104.1/extensions/cpsection/language/model.py	2015-05-09 08:18:33.322564973 +1000
+++ sugar-91785c8/extensions/cpsection/language/model.py	2015-05-08 18:24:32.428245406 +1000
@@ -119,6 +119,7 @@
             langlist = lang.split(':')
         elif line.startswith('LANG='):
             lang = line[5:].replace('"', '')
+            lang = lang.strip()
             if lang.endswith('UTF-8'):
                 lang = lang.replace('UTF-8', 'utf8')
 
EOF
patch -f /usr/lib/python2.7/site-packages/jarabe/controlpanel/gui.py <<EOF || true
diff -ru sugar-v0.104.1/src/jarabe/controlpanel/gui.py sugar-91785c8/src/jarabe/controlpanel/gui.py
--- sugar-v0.104.1/src/jarabe/controlpanel/gui.py	2015-05-09 08:18:33.534561171 +1000
+++ sugar-91785c8/src/jarabe/controlpanel/gui.py	2015-05-08 18:24:32.428245406 +1000
@@ -207,6 +207,14 @@
         self.grab_focus()
 
     def __key_press_event_cb(self, window, event):
+        if event.keyval == Gdk.KEY_Escape:
+            if self._toolbar == self._main_toolbar:
+                self.__stop_clicked_cb(None)
+                self.destroy()
+            else:
+                self.__cancel_clicked_cb(None)
+            return True
+
         # if the user clicked out of the window - fix SL #3188
         if not self.is_active():
             self.present()
@@ -323,6 +331,9 @@
         self._show_main_view()
 
     def __accept_clicked_cb(self, widget):
+        if hasattr(self._section_view, "apply"):
+            self._section_view.apply()
+
         if self._section_view.needs_restart:
             self._section_toolbar.accept_button.set_sensitive(False)
             self._section_toolbar.cancel_button.set_sensitive(False)
EOF
patch -f /usr/lib/python2.7/site-packages/jarabe/model/neighborhood.py <<EOF || true
diff -ru sugar-v0.104.1/src/jarabe/model/neighborhood.py sugar-91785c8/src/jarabe/model/neighborhood.py
--- sugar-v0.104.1/src/jarabe/model/neighborhood.py	2015-05-09 08:18:33.710558018 +1000
+++ sugar-91785c8/src/jarabe/model/neighborhood.py	2015-05-08 18:24:32.428245406 +1000
@@ -417,7 +417,7 @@
             error_handler=self.__set_current_activity_error_cb)
 
     def __set_current_activity_cb(self):
-        logging.warning('_Account.__set_current_activity_cb')
+        logging.debug('_Account.__set_current_activity_cb')
 
     def __set_current_activity_error_cb(self, error):
         logging.debug('_Account.__set_current_activity__error_cb %r', error)
EOF
patch -f /usr/lib/python2.7/site-packages/jarabe/model/network.py <<EOF || true
diff -ru sugar-v0.104.1/src/jarabe/model/network.py sugar-91785c8/src/jarabe/model/network.py
--- sugar-v0.104.1/src/jarabe/model/network.py	2015-05-09 08:18:33.710558018 +1000
+++ sugar-91785c8/src/jarabe/model/network.py	2015-05-08 18:24:32.428245406 +1000
@@ -203,6 +203,7 @@
 _nm_settings = None
 _secret_agent = None
 _connections = None
+_interfaces = None
 
 _nm_device_state_reason_description = None
 
@@ -861,6 +862,24 @@
         self._connections.remove(connection)
 
 
+def get_wireless_interfaces():
+    global _interfaces
+    if _interfaces is None:
+
+        _interfaces = []
+        bus = dbus.SystemBus()
+        for device_path in get_manager().GetDevices():
+            device_object = bus.get_object(NM_SERVICE, device_path)
+            properties = dbus.Interface(device_object,
+                                        'org.freedesktop.DBus.Properties')
+            device_type = properties.Get(NM_DEVICE_IFACE, 'DeviceType')
+            if device_type != NM_DEVICE_TYPE_WIFI:
+                continue
+
+            _interfaces.append(properties.Get(NM_DEVICE_IFACE, 'Interface'))
+    return _interfaces
+
+
 def get_connections():
     global _connections
     if _connections is None:
EOF
patch -f /usr/lib/python2.7/site-packages/jarabe/model/shell.py <<EOF || true
diff -ru sugar-v0.104.1/src/jarabe/model/shell.py sugar-91785c8/src/jarabe/model/shell.py
--- sugar-v0.104.1/src/jarabe/model/shell.py	2015-05-09 08:18:33.710558018 +1000
+++ sugar-91785c8/src/jarabe/model/shell.py	2015-05-08 18:24:32.472244822 +1000
@@ -429,6 +429,8 @@
         self._maximum_open_activities = settings.get_int(
             'maximum-number-of-open-activities')
 
+        self._launch_timers = {}
+
     def get_launcher(self, activity_id):
         return self._launchers.get(str(activity_id))
 
@@ -732,10 +734,13 @@
 
         self.emit('launch-started', home_activity)
 
-        # FIXME: better learn about finishing processes by receiving a signal.
-        # Now just check whether an activity has a window after ~90sec
-        GObject.timeout_add_seconds(90, self._check_activity_launched,
-                                    activity_id)
+        if activity_id in self._launch_timers:
+            GObject.source_remove(self._launch_timers[activity_id])
+            del self._launch_timers[activity_id]
+
+        timer = GObject.timeout_add_seconds(90, self._check_activity_launched,
+                                            activity_id)
+        self._launch_timers[activity_id] = timer
 
     def notify_launch_failed(self, activity_id):
         home_activity = self.get_activity_by_id(activity_id)
@@ -752,6 +757,7 @@
                           activity_id)
 
     def _check_activity_launched(self, activity_id):
+        del self._launch_timers[activity_id]
         home_activity = self.get_activity_by_id(activity_id)
 
         if not home_activity:

EOF
