$LOAD_PATH << 'lib'

require 'gtk2'
require 'indicate'

s = Indicate::Server.new  do |s|
    puts "displayed!"
end

s.type         = Indicate::Type::Message::Mail
s.desktop_file = 'mailman.desktop'

Indicate::Indicator.new("sean.anastasi@gmail.com", 15) do |i|
    puts "indicator displayed!"
    i.hide
end.show!

Gtk.main
