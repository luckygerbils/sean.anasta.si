# encoding: utf-8
#
# Gnome keyring API.
#
# Author::      Sean Anastasi  (mailto:spa@uw.edu)
# Date::        29 Aug 2010
require 'gnome_keyring_bind'
class GnomeKeyring
    extend Enumerable
    
    # defined in c
    # def self.list
    # def self.lock_all
    # def self.available?() end
    
    def self.default
        name = self.default_keyring_name
        return name && GnomeKeyring.new(name)
    end

    def self.default=(keyring)
        self.default_keyring_name = keyring.to_str
    end
    
    def self.each
        self.list.each do |keyring_name|
            yield GnomeKeyring[keyring_name]
        end
    end
    
    def self.[](keyring_name)
        GnomeKeyring.new(keyring_name)
    end
    
    def self.find_items(type, attributes={}) end
    def self.find_network_password(user=nil, domain=nil, server=nil, object=nil, protocol=nil, authtype=nil, port=nil) end
    
    def initialize(keyring_name)
        @name = keyring_name
    end
    
    def name
        return @name
    end
    
    def to_str
        @name
    end
    
    def delete() end
    def change_password(old_password=nil, new_password=nil) end
    
    def set_network_password(user=nil, domain=nil, server=nil, object=nil, protocol=nil, authtype=nil, port=nil) end
    
    class Item
        def initialize(id, keyring)
            @id, @keyring = id, keyring
        end
        
        def delete() end
        
        class Type
            GENERIC_SECRET           = 0
            NETWORK_PASSWORD         = 1
            NOTE                     = 2
            CHAINED_KEYRING_PASSWORD = 3
            ENCRYPTION_KEY_PASSWORD  = 4
            ITEM_PK_STORAGE          = 100
        end
        
        def type=(type) end
        
        def secret=(secret) end
        
        def name=(name) end
        
        def modified() end
        def created() end
        
        # Attributes
        # def attributes
        def []=(key, value)
        end
        
        def [](key)
            attributes[key]
        end
    end
    
    private
    
    # defined in c
    # def self.default_keyring_name
    # def self.default_keyring_name=(keyring_name)
end

