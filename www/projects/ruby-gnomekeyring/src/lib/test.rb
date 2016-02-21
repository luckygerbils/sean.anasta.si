#! /usr/bin/ruby

require 'gnome_keyring'

p GnomeKeyring['login'].locked?

GnomeKeyring.each do |keyring|
    keyring.unlock if keyring.locked?
    puts keyring.name
    keyring.items.each do |item|
        puts "-"*50
        puts "#{item.name} => #{item.secret.inspect}"
        p item.attributes
    end
end
