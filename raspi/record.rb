require 'serialport'
require 'timeout'
require 'fileutils'

LOCK_FILE = "/tmp/serialport-lock"

FileUtils.touch LOCK_FILE

timeout(10) do
  File.open(LOCK_FILE) do |f|
    f.flock(File::LOCK_EX)
  end
end

puts "raspi$ ## reading data from serialport ##"
sp = SerialPort.new('/dev/ttyAMA0', 9600, 8, 1, 0)
sp.read_timeout = 5000

sp.puts
sp.gets

sp.puts "temp_read"
puts sp.gets
temp = sp.gets
puts temp
hum = sp.gets
puts hum

sp.puts "bri_read"
puts sp.gets
bri = sp.gets
puts bri

sp.close

FileUtils.rm LOCK_FILE

time = Time.now.to_i

temp =~ /TMP: ([0-9]+)/
  temp = ($1.to_f/10).to_s

hum =~ /HUM: ([0-9]+)/
  hum = ($1.to_f/10).to_s

bri =~ /BRI: ([0-9]+)/
  bri = $1.to_i.to_s

cmd = "ssh cubik \"cd codes/house; ruby record.rb --time=#{time} --temp=#{temp} --hum=#{hum} --bri=#{bri}\""
puts "raspi$ " +  cmd
system cmd

