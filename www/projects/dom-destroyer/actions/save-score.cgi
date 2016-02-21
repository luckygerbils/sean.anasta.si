#! /usr/bin/ruby
# Save the user's score to the database.
require 'stringio'

res = StringIO.new
begin

    require 'rubygems'
    require 'sequel'
    require 'cgi'

    cgi = CGI.new

    unless cgi.has_key? 'score'
        puts "Missing score parameter."
        exit
    end

    ip    = ENV['REMOTE_ADDR']
    score = cgi['score'].to_i

    Sequel.mysql('dom_destroyer', :host=>'localhost',
        :user=>'root', :password=>'s@^b7AK#') do |db|
        db[:scores].replace(nil, ip, score)
    end
    
    puts "Content-type: application/json"
    puts
    puts "{ip: \"#{ip}\", score: #{score}}"
    
rescue Exception => e
    puts "Content-type: text/plain"
    puts
    puts e.message
    puts e.backtrace.join("\n")
end
