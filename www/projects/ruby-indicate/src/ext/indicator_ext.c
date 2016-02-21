/**
 * Ruby libindicate binding. Indication definition.
 *
 * Author::      Sean Anastasi  (mailto:spa@uw.edu)
 * Date::        19 Sep 2010
 *
 * module Indicate
 *   class Indicator
 */
#include <ruby.h>
#include <glib.h>
#include <libindicate/server.h>

#include "indicator_ext.h"

static VALUE method_Indicator_initialize (const VALUE self);
static VALUE method_Indicator_show (const VALUE self);
static VALUE method_Indicator_hide (const VALUE self);
static VALUE method_Indicator_set_time (const VALUE self, const VALUE value);
static VALUE method_Indicator_set_count (const VALUE self, const VALUE value);
static VALUE method_Indicator_set_sender (const VALUE self, const VALUE value);
static VALUE method_Indicator_set_draw_attention (const VALUE self, const VALUE value);

static VALUE Indicator;

struct Indicate {
    IndicateIndicator *indicator;
};

void Init_indicator_ext ()
{
    g_type_init(); // Initialize glib types if not done yet.
    
    Indicator = rb_define_class_under (rb_define_module ("Indicate"), "Indicator", rb_cObject);
    rb_define_private_method (Indicator, "bind_initialize",         method_Indicator_initialize,          0);
    rb_define_private_method (Indicator, "bind_show",               method_Indicator_show,                0);
    rb_define_private_method (Indicator, "bind_hide",               method_Indicator_hide,                0);
    rb_define_private_method (Indicator, "bind_set_time",           method_Indicator_set_time,            1);
    rb_define_private_method (Indicator, "bind_set_count",          method_Indicator_set_count,           1);
    rb_define_private_method (Indicator, "bind_set_sender",         method_Indicator_set_sender,          1);
    rb_define_private_method (Indicator, "bind_set_draw_attention", method_Indicator_set_draw_attention,  1);
}

static void
display_callback (const IndicateIndicator *indicator, const guint timestamp, const VALUE self)
{
    rb_funcall (self, rb_intern("display_callback"), 0);
}

/**
 * def bind_initialize
 */
static VALUE
method_Indicator_initialize (const VALUE self)
{
    struct Indicate* indicate;
    VALUE indicator;
    
    indicate = ALLOC(struct Indicate);
    indicate->indicator = indicate_indicator_new ();
    indicator = Data_Wrap_Struct (Indicator, 0, free, indicate);
    rb_ivar_set (self, rb_intern("@indicator"), indicator);
    
    g_signal_connect (G_OBJECT (indicate->indicator),
        INDICATE_INDICATOR_SIGNAL_DISPLAY, G_CALLBACK (display_callback), (gpointer) self);
    return Qnil;
}

/**
 * def show
 */
static VALUE
method_Indicator_show (const VALUE self)
{
    struct Indicate *indicate;
    VALUE indicator = rb_ivar_get (self, rb_intern("@indicator"));
    Data_Get_Struct(indicator, struct Indicate, indicate);
    
    indicate_indicator_show (indicate->indicator);
    return Qnil;
}


/**
 * def set_draw_attention
 */
static VALUE
method_Indicator_set_draw_attention (const VALUE self, const VALUE value)
{
    struct Indicate *indicate;
    VALUE indicator = rb_ivar_get (self, rb_intern("@indicator"));
    Data_Get_Struct(indicator, struct Indicate, indicate);
    
    VALUE draw_attention = rb_funcall (value, rb_intern("to_s"), 0);
    indicate_indicator_set_property (indicate->indicator, "draw-attention", StringValuePtr (draw_attention));
    indicate_indicator_show (indicate->indicator);
    return Qnil;
}

/**
 * def hide
 */
static VALUE
method_Indicator_hide (const VALUE self)
{
    struct Indicate *indicate;
    VALUE indicator = rb_ivar_get (self, rb_intern("@indicator"));
    Data_Get_Struct(indicator, struct Indicate, indicate);
    
    indicate_indicator_hide (indicate->indicator);
    return Qnil;
}


/**
 * def set_time(time)
 */
static VALUE
method_Indicator_set_time (const VALUE self, VALUE value)
{
    struct Indicate *indicate;
    VALUE indicator = rb_ivar_get (self, rb_intern("@indicator"));
    Data_Get_Struct(indicator, struct Indicate, indicate);
    
    GTimeVal time;
    g_time_val_from_iso8601 (StringValuePtr (value), &time);
    
    indicate_indicator_set_property_time (indicate->indicator, "time", &time);
    return Qnil;
}

/**
 * def set_count(time)
 */
static VALUE
method_Indicator_set_count (const VALUE self, VALUE value)
{
    struct Indicate *indicate;
    VALUE indicator = rb_ivar_get (self, rb_intern("@indicator"));
    Data_Get_Struct(indicator, struct Indicate, indicate);
    
    VALUE count = rb_funcall (value, rb_intern("to_s"), 0);
    indicate_indicator_set_property (indicate->indicator, "count", StringValuePtr (count));
    return Qnil;
}


/**
 * def set_sender(sender)
 */
static VALUE
method_Indicator_set_sender (const VALUE self, VALUE value)
{
    struct Indicate *indicate;
    VALUE indicator = rb_ivar_get (self, rb_intern("@indicator"));
    Data_Get_Struct(indicator, struct Indicate, indicate);
    
    indicate_indicator_set_property (indicate->indicator, "sender", StringValuePtr (value));
    return Qnil;
}


