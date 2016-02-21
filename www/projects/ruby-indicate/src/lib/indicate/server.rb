# encoding: utf-8
#
# Ruby libindicate binding. Server class.
#
# Author::      Sean Anastasi  (mailto:spa@uw.edu)
# Date::        19 Sep 2010
#
module Indicate
    class Server
        attr_reader :type, :desktop_file
        
        #
        # Indicate::Server.new
        #
        # Indicate::Server.new(
        #   :type         => 'message.im',
        #   :desktop_file => 'application.desktop')
        #
        def initialize(options={}, &block)
            bind_initialize
            self.type         = options[:type]         if options[:type]
            self.desktop_file = options[:desktop_file] if options[:desktop_file]
            self.display(&block) if block_given?
        end
        
        #
        # Set the indicate type, one of Indicate::Type
        #
        # s.type = Indicate::Type::Message::Mail
        #
        def type=(type)
            @type = type
            bind_set_type(@type)
        end
        
        #
        # Set the indicate type, one of Indicate::Type
        #
        # s.desktop_file = 'application.desktop'
        #
        def desktop_file=(filename)
            raise "File not found #{filename}." unless File.exists?(filename)
            
            filename = File.expand_path(filename)
            @desktop_file = filename
            bind_set_desktop_file(@desktop_file)
        end
        
        #
        # Add a block to be called when this 
        #
        def display(&block)
            raise "block required" unless block_given?
            @display_callbacks ||= []
            @display_callbacks << block
        end
        
        private
        def display_callback
            @display_callbacks.each{|c| c.call(self)}
        end
    end
end
