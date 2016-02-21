# encoding: utf-8
#
# Ruby extension configuration for libindicate bindings.
#
# Author::      Sean Anastasi  (mailto:spa@uw.edu)
# Date::        19 Sep 2010
require 'mkmf'

EXTENSION_NAME = 'indicate_bind'

pkg_config 'glib-2.0'
pkg_config 'indicate'

dir_config EXTENSION_NAME

create_makefile EXTENSION_NAME
