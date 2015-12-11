require 'serialport'
require 'timeout'
require 'fileutils'
require 'net/http'
require 'uri'

LOCK_FILE = "/tmp/serialport-lock"
PROMPT = "RasPi-ExtBoard> "

BASIC_PATH = File.dirname(__FILE__) + "/basic.txt"

def sp_read(sp)
  out = ""
  cont = 0
  while true
    begin
      buf = sp.read_nonblock(1)
    rescue Errno::EAGAIN
      cont += 1
      if cont > 5
        return out
      end
      sleep 0.1
      next
    end
    cont = 0
    out << buf
    if buf == "\n"
      return out
    end
  end
end

def command(sp,str)

  result = ""

  timeout(10) do

    sp.puts str
    while(true)
      line = sp_read(sp)
      STDOUT.print line if line != "\r\n"
      if line != PROMPT
        result << line
      else
        break
      end
    end

  end

  return result

end

def main

  FileUtils.touch LOCK_FILE

  timeout(10) do
    File.open(LOCK_FILE) do |f|
      f.flock(File::LOCK_EX)
    end
  end

  puts "## opening serial port ##"
  sp = SerialPort.new('/dev/ttyAMA0', 9600, 8, 1, 0)
  sp.read_timeout = 5000

  command(sp,"")
  temp_str = command(sp,"temp_read")
  bri_str = command(sp,"bri_read")

  sp.close
  FileUtils.rm LOCK_FILE
  puts
  puts "## closed serial port ##"

  if temp_str =~ /TMP: ([0-9]+)/
    tmp = ($1.to_f/10)
  else
    puts "TMP not found"
    exit -1
  end

  if temp_str =~ /HUM: ([0-9]+)/
    hum = ($1.to_f/10)
  else
    puts "HUM not found"
    exit -1
  end

  if bri_str =~ /BRI: ([0-9]+)/
    bri = $1.to_i
  else
    puts "BRI not found"
    exit -1
  end

  time = Time.now.to_i
  
  puts "temperature: #{tmp}"
  puts "humidity   : #{hum}"
  puts "brightness : #{bri}"
  puts "time       : #{time}"

  basic_user, basic_pass = File.open(BASIC_PATH){|f| f.gets.chomp }.split(":",2)

  res = Net::HTTP.post_form(
    URI.parse("http://#{basic_user}:#{basic_pass}@home.cubik.jp/api/v1/envs"),
    {
      temperature: tmp,
      humidity: hum,
      brightness: bri,
      time: time
    }
  )

  puts res.body
  return

end

main
