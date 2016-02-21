# encoding: utf-8
#
# Ruby libindicate binding. Indicator class.
#
# Author::      Sean Anastasi  (mailto:spa@uw.edu)
# Date::        19 Sep 2010
#
module Indicate
    class Indicator
        attr_reader :sender, :data
    
        #
        # # Shows a message from Test Application
        # Indicate::Indicator.new("Test Application").show
        #
        # # Shows an urgent message from Test Application with an elapsed time indication.
        # Indicate::Indicator.new("Test Application", Time.now).show!
        #
        # # Shows an urgent message from Test Application with a count indication.
        # Indicate::Indicator.new("Test Application", 15).show!
        #
        def initialize(sender, data=nil, &block)
            bind_initialize
            self.sender = sender
            self.data = data if data
            self.display(&block) if block_given?
        end
        
        #
        # Show this indicator.
        #
        def show
            bind_show
        end
        
        #
        # Hide this indicator.
        #
        def hide
            bind_hide
        end
        
        #
        # Show this indicator, drawing attention to the indicator panel.
        #
        def show!
            bind_set_draw_attention(true)
            show
        end
        
        #
        # Set the data to go with this indicator.
        # Can be a Time or Integer
        #
        def data=(data)
            @data = data
            if @data.is_a? Integer
                bind_set_count(data)
            elsif @data.is_a? Time
                bind_set_time(data.iso8601)
            end
        end
        
        def sender=(sender)
            @sender = sender
            bind_set_sender(@sender)
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
