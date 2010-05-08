#!/usr/bin/env ruby

require 'pty'
require 'logger'

class UnNetHack

def initialize()
	ENV["NETHACKOPTIONS"]="windowtype:dummy"

	raise "Environment variable HACKDIR is not set" if not ENV["HACKDIR"]

	@logger = Logger.new "logs/unnethack.log", "daily"

	@stdin, @stdout, @pid = PTY.spawn("#{ENV['HACKDIR']}/unnethack -D")
	@logger.info @pid

	l=""
	while (not l.include?("nh_poskey")) do
		l=@stdin.readline
		@logger.info l.strip
		if (l.include?("Do you want to keep the save file")) then
			@stdout.puts 'n'
		elsif (l.include?("getlin")) then
			@stdout.puts "\n"
		elsif (l.include?("Destroy old game")) then
			@stdout.puts "n"
		elsif (l =~ /Window type dummy not recognized/) then
			@stdout.puts "\x16\x3" # ctrl-c
			raise "Window type 'dummy' not compiled into UnNetHack"
		end
	end
	@stdin.sync = true
	@stdout.sync = true
	$stdin.sync = true
	$stdout.sync = true
end

# Reads until it encounters str
def read_until(str)
	l=""
	while (not l.include?(str)) do
		l=@stdin.readline
		@logger.info l.strip
	end
	sleep 0.01
	return l
end

# Quits the current game
def quit
	@stdout.puts '#'
	read_until("ext_cmd")
	@stdout.puts 'quit'

	read_until("Really quit?")
	@stdout.puts 'y'
	l=@stdin.readline
	read_until("Dump core")
	@stdout.puts 'n'

	begin
		read_until("XXX")
	rescue Errno::EIO
		@logger.warn "Errno::EIO"
	end
	@logger.info "Waiting for process #{@pid}"
	Process.waitpid(@pid, Process::WNOHANG)
	@logger.info "Finished"
	sleep 0.1
end

# Teleports to the specified level by name or number
def teleport_to(level)
	@stdout.puts "\x16\x16" # ctrl-v
	@stdout.flush
	@stdout.puts level
	read_until("nh_poskey")
end

# Wishes for an item and returns the name of the received item
def wish_for(wish)
	@stdout.puts "\x16\x17" # ctrl-w
	@stdout.flush
	read_until("For what do you wish")
	@stdout.puts wish
	read_until("dummy_raw_print")
	return @stdin.readline.strip
end

end

