require 'serialport'
require 'timeout'
require 'fileutils'
require 'net/http'
require 'uri'
require 'json'
require 'datadog/statsd'

LOCK_FILE = "/tmp/serialport-lock"
PROMPT = "RasPi-ExtBoard> "

BASIC_PATH = File.dirname(__FILE__) + "/basic.txt"
IR_PATTERN_PATH = File.dirname(__FILE__) + "/ir_pattern.json"

BASIC_USER, BASIC_PASS = File.open(BASIC_PATH){|f| f.gets.chomp }.split(":",2)

def sp_lock
  FileUtils.touch LOCK_FILE
  timeout(10) do
    File.open(LOCK_FILE) do |f|
      f.flock(File::LOCK_EX)
    end
  end
  yield
ensure
  FileUtils.rm LOCK_FILE
end

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


def sp_command(sp, str)

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
  update_envs
  control_ac
end

def update_envs

  puts "#### update envs ####"

  time = Time.now.to_i

  temp_str = ""
  bri_str = ""

  sp_lock do

    puts "## opening serial port ##"
    sp = SerialPort.new('/dev/ttyAMA0', 9600, 8, 1, 0)
    sp.read_timeout = 5000

    begin

      sp_command(sp,"")
      temp_str = sp_command(sp,"temp_read")
      bri_str = sp_command(sp,"bri_read")

    ensure
      sp.close
      puts "## closed serial port ##"
    end

  end

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

  puts "time       : #{time}"
  puts "temperature: #{tmp}"
  puts "humidity   : #{hum}"
  puts "brightness : #{bri}"


  res = Net::HTTP.post_form(
    URI.parse("https://#{BASIC_USER}:#{BASIC_PASS}@home.cubik.jp/api/v1/envs"),
    {
      temperature: tmp,
      humidity: hum,
      brightness: bri,
      time: time
    }
  )

  puts res.body

  ## send to datadog
  begin
    statsd = Datadog::Statsd.new('localhost', 8125)
    statsd.gauge('home.env.temperature', tmp)
    statsd.gauge('home.env.humidity', hum)
    statsd.gauge('home.env.brightness', bri)
  rescue => e
    p e
  end

  return

end

def control_ac

  puts "#### control AC ####"

  #### get commands from server ####
  
  base_uri = "https://home.cubik.jp/api/v1/ac/commands"

  get_uri = URI.parse(base_uri)
  http = Net::HTTP.new(get_uri.host, get_uri.port)
  http.use_ssl = true
  req = Net::HTTP::Get.new(get_uri.path)
  res = http.start do |x|
    x.request(req)
  end
  cmd_data = JSON.parse(res.body)

  p cmd_data

  return if cmd_data.length == 0

  cmd_data.sort!{|a,b| a['id'] <=> b['id']}

  cmd_last = cmd_data.last
  cmd_type = cmd_last['command']

  #### send ir pattern to AC ####
  
  ir_data = JSON.parse(File.open(IR_PATTERN_PATH){|f|f.read})
  interval = ir_data['ac'][cmd_type]['interval']
  pattern = ir_data['ac'][cmd_type]['pattern']

  sp_lock do

    puts "## opening serial port ##"
    sp = SerialPort.new('/dev/ttyAMA0', 9600, 8, 1, 0)
    sp.read_timeout = 5000

    begin

      sp_command(sp,"")
      cmd = "ir_send #{interval}\n#{pattern}"
      cmd_result = sp_command(sp, cmd)

    ensure
      sp.close
      puts "## closed serial port ##"
    end

  end

  #### delete commands on server ####

  cmd_data.each do |c|

    delete_uri = URI.parse("#{base_uri}/#{c['id']}")
    req = Net::HTTP::Delete.new(delete_uri.path)
    req.basic_auth(BASIC_USER, BASIC_PASS)
    res = http.start do |x|
      x.request(req)
    end
    puts res.body
  end

end
main
