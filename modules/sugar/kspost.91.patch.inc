# FIXME: waiting for a sugar release after sugar-0.102.0-1.fc20.noarch.rpm
patch /usr/share/sugar/extensions/cpsection/backup/view.py <<EOF || true
diff --git a/extensions/cpsection/backup/view.py b/extensions/cpsection/backup/view.py
index f0fe580..ecfca66 100644
--- a/extensions/cpsection/backup/view.py
+++ b/extensions/cpsection/backup/view.py
@@ -324,6 +324,7 @@ class OperationPanel(Gtk.Grid):
         self._confirm_restore_chkbtn.show()
         self._options_combo.hide()
         self._continue_btn.set_label(_('Confirm'))
+        self._continue_btn.show()
         if self._continue_btn_handler_id != 0:
             self._continue_btn.disconnect(self._continue_btn_handler_id)
         self._continue_btn_handler_id = self._continue_btn.connect(
EOF

# FIXME: waiting for activity release after ImageViewer-59
patch /home/olpc/Activities/ImageViewer.activity/ImageView.py << EOF || true
diff --git a/ImageView.py b/ImageView.py
index d7bee8f..16a9de8 100644
--- a/ImageView.py
+++ b/ImageView.py
@@ -31,9 +31,8 @@ ZOOM_MIN = 0.05
 
 def _surface_from_file(file_location, ctx):
     pixbuf = GdkPixbuf.Pixbuf.new_from_file(file_location)
-    surface = ctx.get_target().create_similar(
-        cairo.CONTENT_COLOR_ALPHA, pixbuf.get_width(),
-        pixbuf.get_height())
+    surface = cairo.ImageSurface(cairo.FORMAT_ARGB32,
+                                 pixbuf.get_width(), pixbuf.get_height())
 
     ctx_surface = cairo.Context(surface)
     Gdk.cairo_set_source_pixbuf(ctx_surface, pixbuf, 0, 0)
EOF
