/**
 * Ruby libindicate binding. Server definition.
 *
 * Author::      Sean Anastasi  (mailto:spa@uw.edu)
 * Date::        19 Sep 2010
 *
 * module Indicate
 *   class Server
 */
#include <ruby.h>
#include <glib.h>
#include <libindicate/server.h>

#include "server_ext.h"

static VALUE method_Server_initialize (const VALUE self);
static VALUE method_Server_set_type (const VALUE self, const VALUE type);
static VALUE method_Server_set_desktop_file (const VALUE self, const VALUE filename);

static VALUE Server;

struct Indicate {
    IndicateServer *server;
};


void Init_server_ext ()
{
    g_type_init(); // Initialize glib types if not done yet.
    
    Server = rb_define_class_under (rb_define_module ("Indicate"), "Server", rb_cObject);
    rb_define_private_method (Server, "bind_initialize",       method_Server_initialize,       0);
    rb_define_private_method (Server, "bind_set_type",         method_Server_set_type,         1);
    rb_define_private_method (Server, "bind_set_desktop_file", method_Server_set_desktop_file, 1);
}


static void
display_callback (const IndicateIndicator *indicator, const guint timestamp, const VALUE self)
{
    rb_funcall (self, rb_intern("display_callback"), 0);
}

/**
 * def initialize(options={})
 *   @server = indicate_server_ref_default ()
 * end
 */
static VALUE
method_Server_initialize (const VALUE self)
{
    struct Indicate* indicate;
    VALUE server;
    
    indicate = ALLOC(struct Indicate);
    indicate->server = indicate_server_ref_default ();
    server = Data_Wrap_Struct(Server, 0, free, indicate);
    rb_ivar_set (self, rb_intern("@server"), server);
    
    g_signal_connect (G_OBJECT (indicate->server),
        INDICATE_SERVER_SIGNAL_SERVER_DISPLAY, G_CALLBACK (display_callback), (gpointer) self);
    
    indicate_server_show (indicate->server);
    return Qnil;
}

/**
 * def type=(type)
 *   @server.set_type(type)
 * end
 */
static VALUE
method_Server_set_type (const VALUE self, VALUE type)
{
    struct Indicate *indicate;
    VALUE server = rb_ivar_get (self, rb_intern("@server"));
    Data_Get_Struct(server, struct Indicate, indicate);
    
    indicate_server_set_type (indicate->server, StringValuePtr (type));
    return Qnil;
}

/**
 * def desktop_file=(filename)
 *   @server.set_desktop_file(filename)
 * end
 */
static VALUE
method_Server_set_desktop_file (const VALUE self, VALUE filename)
{
    struct Indicate *indicate;
    VALUE server = rb_ivar_get (self, rb_intern("@server"));
    Data_Get_Struct(server, struct Indicate, indicate);
    
    indicate_server_set_desktop_file (indicate->server, StringValuePtr (filename));    
    
    return Qnil;
}

