require 'serialport'

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

time = Time.now.to_i

temp =~ /TMP: ([0-9]+)/
temp = ($1.to_f/10).to_s

hum =~ /HUM: ([0-9]+)/
hum = ($1.to_f/10).to_s

bri =~ /BRI: ([0-9]+)/
bri = $1.to_i.to_s

system "ssh cubik ruby codes/house/record.rb --time=#{time} --temp=#{temp} --hum=#{hum} --bri=#{bri}"

