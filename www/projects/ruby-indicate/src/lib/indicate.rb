# encoding: utf-8
#
# Ruby libindicate binding.
#
# Author::      Sean Anastasi  (mailto:spa@uw.edu)
# Date::        19 Sep 2010
#
module Indicate
    VERSION = [0, 1, 0]
    
    module Type
        module Message
            IM      = "message.im"
            Mail    = "message.mail"
            Instant = "message.instant"
            Micro   = "message.micro"
        end
        
        module System
        end
        
        module Media
        end
    end
end

require 'indicate/server'
require 'indicate/indicator'
require 'indicate_bind'

