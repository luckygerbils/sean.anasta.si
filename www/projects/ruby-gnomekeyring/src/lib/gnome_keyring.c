/**
 * Ruby Gnome keyring bindings.
 *
 * Author:      Sean Anastasi  (mailto:sean.anastasi@gmail.com)
 * Date:        19 Sep 2010
 */
#include <ruby.h>
#include <glib.h>
#include <gnome-keyring.h>

VALUE GnomeKeyring;
VALUE GnomeKeyringItem;

static void raise_error_unless_ok (const GnomeKeyringResult result);

static VALUE class_method_GnomeKeyring_list (const VALUE klass);
static VALUE class_method_GnomeKeyring_is_available (const VALUE klass);
static VALUE class_method_GnomeKeyring_lock_all (const VALUE klass);
static VALUE class_method_GnomeKeyring_create_sync (const int argc, VALUE *argv, const VALUE klass);
static VALUE class_method_GnomeKeyring_create_async (const int argc, VALUE *argv, const VALUE klass);

static VALUE method_GnomeKeyring_lock (const VALUE self);
static VALUE method_GnomeKeyring_unlock (const int argc, VALUE *argv, const VALUE self);

static VALUE method_GnomeKeyring_get_lock_on_idle (const VALUE self);
static VALUE method_GnomeKeyring_set_lock_on_idle (const VALUE self, const VALUE value);
static VALUE method_GnomeKeyring_get_lock_timeout (const VALUE self);
static VALUE method_GnomeKeyring_set_lock_timeout (const VALUE self, const VALUE value);

static VALUE method_GnomeKeyring_get_mtime (const VALUE self);
static VALUE method_GnomeKeyring_get_ctime (const VALUE self);
static VALUE method_GnomeKeyring_get_locked (const VALUE self);

static VALUE class_method_GnomeKeyring_get_default_keyring_name (const VALUE klass);
static VALUE class_method_GnomeKeyring_set_default_keyring_name (const VALUE klass, VALUE keyring_name);

static VALUE method_GnomeKeyring_create_item (const int argc, VALUE *argv, const VALUE self);
static VALUE method_GnomeKeyring_items (const VALUE self);



static VALUE method_GnomeKeyringItem_name (const VALUE self);
static VALUE method_GnomeKeyringItem_secret (const VALUE self);
static VALUE method_GnomeKeyringItem_type (const VALUE self);
static VALUE method_GnomeKeyringItem_get_attributes (const VALUE self);

/**
 * Initialize classes and method bindings.
 */
void Init_gnome_keyring_bind ()
{
    GnomeKeyring = rb_define_class ("GnomeKeyring", rb_cObject);
    GnomeKeyringItem = rb_define_class_under (GnomeKeyring, "Item", rb_cObject);
    
    rb_define_singleton_method (GnomeKeyring, "list", class_method_GnomeKeyring_list, 0);
    rb_define_singleton_method (GnomeKeyring, "available?", class_method_GnomeKeyring_is_available, 0);
    rb_define_singleton_method (GnomeKeyring, "lock_all", class_method_GnomeKeyring_lock_all, 0);
    rb_define_singleton_method (GnomeKeyring, "create", class_method_GnomeKeyring_create_sync, -1);
    rb_define_singleton_method (GnomeKeyring, "create!", class_method_GnomeKeyring_create_async, -1);
    
    rb_define_module_function (GnomeKeyring, "default_keyring_name", class_method_GnomeKeyring_get_default_keyring_name, 0);
    rb_define_module_function (GnomeKeyring, "default_keyring_name=", class_method_GnomeKeyring_set_default_keyring_name, 1);
    
    //
    // Keyring instance methods.
    //
    rb_define_method (GnomeKeyring, "lock", method_GnomeKeyring_lock, 0);
    rb_define_method (GnomeKeyring, "unlock", method_GnomeKeyring_unlock, -1);
    
    rb_define_method (GnomeKeyring, "lock_on_idle?", method_GnomeKeyring_get_lock_on_idle, 0);
    rb_define_method (GnomeKeyring, "lock_on_idle=", method_GnomeKeyring_set_lock_on_idle, 1);
    rb_define_method (GnomeKeyring, "lock_timeout", method_GnomeKeyring_get_lock_timeout, 0);
    rb_define_method (GnomeKeyring, "lock_timeout=", method_GnomeKeyring_set_lock_timeout, 1);
    
    rb_define_method (GnomeKeyring, "modified", method_GnomeKeyring_get_mtime, 0);
    rb_define_method (GnomeKeyring, "created", method_GnomeKeyring_get_ctime, 0);
    rb_define_method (GnomeKeyring, "locked?",  method_GnomeKeyring_get_locked, 0);
    
    rb_define_method (GnomeKeyring, "create_item",  method_GnomeKeyring_create_item, -1);
    rb_define_method (GnomeKeyring, "items",  method_GnomeKeyring_items, 0);
    
    rb_define_method (GnomeKeyringItem, "name", method_GnomeKeyringItem_name, 0);
    rb_define_method (GnomeKeyringItem, "secret", method_GnomeKeyringItem_secret, 0);
    rb_define_method (GnomeKeyringItem, "type", method_GnomeKeyringItem_type, 0);
    rb_define_method (GnomeKeyringItem, "attributes", method_GnomeKeyringItem_get_attributes, 0);
}

static void raise_error_unless_ok (const GnomeKeyringResult result)
{
    if (result == GNOME_KEYRING_RESULT_OK)
        return;
    rb_raise (rb_eRuntimeError, "%s", gnome_keyring_result_to_message (result));
}

//
// Method definitions.
//

/**
 * GnomeKeyring.list
 * => ["session", "login"]
 */
static VALUE class_method_GnomeKeyring_list (const VALUE self)
{
    const VALUE result_list = rb_ary_new();
    GList *names = NULL;
    GnomeKeyringResult result;
    
    result = gnome_keyring_list_keyring_names_sync (&names);
    raise_error_unless_ok (result);

    GList *it = names;
    while (it != NULL)
      {
        rb_ary_push (result_list, rb_str_new2 ((gchar*) it->data));
        it = it->next;
      }

    gnome_keyring_string_list_free (names);
    
    return result_list;
}

/**
 * GnomeKeyring.available?
 * => true
 */
static VALUE class_method_GnomeKeyring_is_available (const VALUE klass)
{
    if (gnome_keyring_is_available ())
        return Qtrue;
    else
        return Qfalse;
}

/**
 * GnomeKeyring.default_keyring_name
 * => "login"
 */
static VALUE class_method_GnomeKeyring_get_default_keyring_name (const VALUE klass)
{
    GnomeKeyringResult result;
    gchar *keyring_name_c = NULL;
    VALUE  keyring_name = Qnil;
    
    result = gnome_keyring_get_default_keyring_sync (&keyring_name_c);
    raise_error_unless_ok (result);
    
    if (keyring_name_c != NULL)
      {
        keyring_name = rb_str_new2 (keyring_name_c);
        g_free (keyring_name_c);
      }

    return keyring_name;
}

/**
 * GnomeKeyring.default_keyring_name = "login"
 */
static VALUE class_method_GnomeKeyring_set_default_keyring_name (const VALUE klass, VALUE keyring_name)
{
    GnomeKeyringResult result;
    result = gnome_keyring_set_default_keyring_sync (StringValuePtr (keyring_name));
    raise_error_unless_ok (result);
    return Qnil;
}

/**
 * GnomeKeyring.lock_all
 */
static VALUE class_method_GnomeKeyring_lock_all (const VALUE klass)
{
    raise_error_unless_ok (gnome_keyring_lock_all_sync ());
    return Qnil;
}

/**
 * GnomeKeyring.create("keyring name")
 * => #<GnomeKeyring>
 *
 *  GnomeKeyring.create("keyring name", "password")
 * => #<GnomeKeyring>
 */
static VALUE class_method_GnomeKeyring_create (const int argc, VALUE *argv, const VALUE klass, const gboolean sync)
{
    GnomeKeyringResult result = GNOME_KEYRING_RESULT_OK;
    VALUE keyring_name;
    VALUE password;
    
    rb_scan_args (argc, argv, "11", &keyring_name, &password);
    
    if (sync)
        result = gnome_keyring_create_sync (StringValuePtr (keyring_name), StringValuePtr (password));
    else
        gnome_keyring_create (StringValuePtr (keyring_name), StringValuePtr (password), NULL, NULL, NULL);
    
    raise_error_unless_ok (result);

    return rb_funcall (klass, rb_intern ("[]"), 1, keyring_name);
}

static VALUE class_method_GnomeKeyring_create_async (const int argc, VALUE *argv, const VALUE klass)
{
    return class_method_GnomeKeyring_create (argc, argv, klass, FALSE);
}

static VALUE class_method_GnomeKeyring_create_sync (const int argc, VALUE *argv, const VALUE klass)
{
    return class_method_GnomeKeyring_create (argc, argv, klass, TRUE);
}

/**
 * GnomeKeyring["login"].lock
 */
static VALUE method_GnomeKeyring_lock (const VALUE self)
{
    VALUE keyring_name = rb_ivar_get (self, rb_intern ("@name"));
    raise_error_unless_ok (gnome_keyring_lock_sync (StringValuePtr (keyring_name)));
    return Qnil;
}

/**
 * GnomeKeyring["login"].unlock
 * GnomeKeyring["login"].unlock("password")
 */
static VALUE method_GnomeKeyring_unlock (const int argc, VALUE *argv, const VALUE self)
{
    VALUE keyring_name = rb_ivar_get (self, rb_intern ("@name"));
    VALUE password = Qnil;
    
    rb_scan_args (argc, argv, "01", &password);
    
    raise_error_unless_ok (
    gnome_keyring_unlock_sync (
        StringValuePtr (keyring_name),
        password != Qnil? StringValuePtr (password): NULL));
    
    
    return Qnil;
}

/**
 * GnomeKeyring["login"].lock_on_idle?
 * => false
 */
static VALUE method_GnomeKeyring_get_lock_on_idle (const VALUE self)
{
    GnomeKeyringInfo *info = NULL;
    VALUE result;
    VALUE keyring_name = rb_ivar_get (self, rb_intern ("@name"));
    
    raise_error_unless_ok (gnome_keyring_get_info_sync (StringValuePtr (keyring_name), &info));
    result = gnome_keyring_info_get_lock_on_idle (info)? Qtrue : Qfalse;
    gnome_keyring_info_free(info);
    return result;
}

/**
 * GnomeKeyring["login"].lock_on_idle = true
 */
static VALUE method_GnomeKeyring_set_lock_on_idle (const VALUE self, const VALUE value)
{
    GnomeKeyringInfo *info = NULL;
    VALUE keyring_name = rb_ivar_get (self, rb_intern ("@name"));
    
    raise_error_unless_ok (gnome_keyring_get_info_sync (StringValuePtr (keyring_name), &info));
    gnome_keyring_info_set_lock_on_idle (info, value? TRUE: FALSE);
    raise_error_unless_ok (gnome_keyring_set_info_sync (StringValuePtr (keyring_name), info));
    gnome_keyring_info_free(info);
    
    return Qnil;
}

/**
 * GnomeKeyring["login"].lock_timeout
 * => 60
 */
static VALUE method_GnomeKeyring_get_lock_timeout (const VALUE self)
{
    GnomeKeyringInfo *info = NULL;
    VALUE keyring_name = rb_ivar_get (self, rb_intern ("@name"));
    
    raise_error_unless_ok (gnome_keyring_get_info_sync (StringValuePtr (keyring_name), &info));
    guint32 timeout = gnome_keyring_info_get_lock_timeout (info);
    gnome_keyring_info_free(info);
    
    return INT2NUM (timeout);
}

/**
 * GnomeKeyring["login"].lock_timeout = 60
 */
static VALUE method_GnomeKeyring_set_lock_timeout (const VALUE self, const VALUE value)
{
    GnomeKeyringInfo *info = NULL;
    VALUE keyring_name = rb_ivar_get (self, rb_intern ("@name"));
    
    raise_error_unless_ok (gnome_keyring_get_info_sync (StringValuePtr (keyring_name), &info));
    gnome_keyring_info_set_lock_timeout (info, NUM2INT (value));
    raise_error_unless_ok (gnome_keyring_set_info_sync (StringValuePtr (keyring_name), info));
    gnome_keyring_info_free(info);
    return Qnil;
}

/**
 * GnomeKeyring["login"].modified
 */
static VALUE method_GnomeKeyring_get_mtime (const VALUE self)
{
    return Qnil;
}

/**
 * GnomeKeyring["login"].created
 */
static VALUE method_GnomeKeyring_get_ctime (const VALUE self)
{
    return Qnil;
}

/**
 * GnomeKeyring["login"].locked?
 */
static VALUE method_GnomeKeyring_get_locked (const VALUE self)
{
    GnomeKeyringInfo *info = NULL;
    VALUE result;
    VALUE keyring_name = rb_ivar_get (self, rb_intern ("@name"));
    
    raise_error_unless_ok (gnome_keyring_get_info_sync (StringValuePtr (keyring_name), &info));
    result = gnome_keyring_info_get_is_locked (info) ? Qtrue : Qfalse;
    gnome_keyring_info_free(info);
    return result;
}

/**
 * GnomeKeyring["login"].create_item(GnomeKeyringItem::GENERIC_SECRET, "item name", {}, "password")
 * => #<GnomeKeyring::Item>
 */
static VALUE method_GnomeKeyring_create_item (const int argc, VALUE *argv, const VALUE self)
{
    VALUE type, display_name, attributes, secret;
    VALUE update = Qfalse;
    VALUE keyring_name = rb_ivar_get (self, rb_intern ("@name"));
    
    rb_scan_args (argc, argv, "41", &type, &display_name, &attributes, &secret, &update);
    
    guint32 item_id;
    
    raise_error_unless_ok (
        gnome_keyring_item_create_sync (
            StringValuePtr (keyring_name), 
            NUM2INT (type),
            StringValuePtr (display_name),
            NULL, // TODO: convert from hash
            StringValuePtr (secret),
            update,
            &item_id));
    
    return rb_funcall (GnomeKeyringItem, rb_intern ("new"), 2, INT2FIX (item_id), self);
}

static VALUE method_GnomeKeyring_items (const VALUE self)
{
    VALUE keyring_name = rb_ivar_get (self, rb_intern ("@name"));
    VALUE items = rb_ary_new ();
    GList *item_ids = NULL;
    
    raise_error_unless_ok (
        gnome_keyring_list_item_ids_sync (
            StringValuePtr (keyring_name),
            &item_ids
        ));
    
    GList *it = item_ids;
    while (it != NULL)
      {
        VALUE item = rb_funcall (GnomeKeyringItem, rb_intern ("new"), 2, INT2FIX (GPOINTER_TO_UINT (it->data)), self);
        rb_ary_push (items, item);
        it = it->next;
      }
    g_list_free (item_ids);
    
    return items;
}

static VALUE method_GnomeKeyringItem_name (const VALUE self)
{
    VALUE keyring_name = rb_ivar_get (rb_ivar_get (self, rb_intern ("@keyring")), rb_intern ("@name"));
    VALUE item_id = rb_ivar_get (self, rb_intern ("@id"));
    VALUE result;
    
    GnomeKeyringItemInfo *info = NULL;
    
    raise_error_unless_ok (
    gnome_keyring_item_get_info_sync (
        StringValuePtr (keyring_name), NUM2INT (item_id), &info));
    
    char *display_name = gnome_keyring_item_info_get_display_name (info);
    result = rb_str_new2 (display_name);

    gnome_keyring_item_info_free (info);
    g_free (display_name);
    
    return result;
}

static VALUE method_GnomeKeyringItem_secret (const VALUE self)
{
    VALUE keyring_name = rb_ivar_get (rb_ivar_get (self, rb_intern ("@keyring")), rb_intern ("@name"));
    VALUE item_id = rb_ivar_get (self, rb_intern ("@id"));
    VALUE result;
    
    GnomeKeyringItemInfo *info = NULL;
    
    raise_error_unless_ok (
    gnome_keyring_item_get_info_sync (
        StringValuePtr (keyring_name), NUM2INT (item_id), &info));
    
    char *secret = gnome_keyring_item_info_get_secret (info);
    result = rb_str_new2 (secret);

    gnome_keyring_item_info_free (info);
    g_free (secret);
    
    return result;
}

static VALUE method_GnomeKeyringItem_type (const VALUE self)
{
    VALUE keyring_name = rb_ivar_get (rb_ivar_get (self, rb_intern ("@keyring")), rb_intern ("@name"));
    VALUE item_id = rb_ivar_get (self, rb_intern ("@id"));
    
    GnomeKeyringItemInfo *info = NULL;
    
    raise_error_unless_ok (
    gnome_keyring_item_get_info_sync (
        StringValuePtr (keyring_name), NUM2INT (item_id), &info));
    
    GnomeKeyringItemType type = gnome_keyring_item_info_get_type (info);

    gnome_keyring_item_info_free (info);
    
    return INT2FIX (type);
}

static VALUE method_GnomeKeyringItem_get_attributes (const VALUE self)
{
    VALUE keyring_name = rb_ivar_get (rb_ivar_get (self, rb_intern ("@keyring")), rb_intern ("@name"));
    VALUE item_id = rb_ivar_get (self, rb_intern ("@id"));
    VALUE attribute_map = rb_hash_new ();
    
    GnomeKeyringAttributeList *attributes = NULL;
    
    raise_error_unless_ok (
    gnome_keyring_item_get_attributes_sync (
        StringValuePtr (keyring_name), NUM2INT (item_id), &attributes));
    
    int i;
    for (i = 0; i < attributes->len; i++)
      {
        GnomeKeyringAttribute attribute = g_array_index (attributes, GnomeKeyringAttribute, i);
        
        if (attribute.type == GNOME_KEYRING_ATTRIBUTE_TYPE_STRING)
          {
            rb_hash_aset (attribute_map, rb_str_new2 (attribute.name), rb_str_new2 (attribute.value.string));
          }
        else // GNOME_KEYRING_ATTRIBUTE_TYPE_UINT32
          {
            rb_hash_aset (attribute_map, rb_str_new2 (attribute.name), INT2FIX (attribute.value.integer));
          }
      }
    
    gnome_keyring_attribute_list_free (attributes);
    return attribute_map;
}




