# encoding: utf-8
#
# Ruby extension configuration for Gnome keyring bindings.
#
# Author::      Sean Anastasi  (mailto:spa@uw.edu)
# Date::        21 Sep 2010
require 'mkmf'

EXTENSION_NAME = 'gnome_keyring_bind'

pkg_config 'glib-2.0'
pkg_config 'gnome-keyring-1'

dir_config EXTENSION_NAME
create_makefile EXTENSION_NAME
