require 'bundler/setup'
require 'sequel'
require 'optparse'

DB_PATH = File.dirname(__FILE__) + "/db.sqlite3"
TEMPLATE_PATH = File.dirname(__FILE__) + "/template.html"
OUTPUT_PATH = "/home/cubing/html/house/index.html"

OPTS = {}
opt = OptionParser.new
opt.on('--time=VAL'){|v| OPTS[:time] = Integer(v) }
opt.on('--temp=VAL'){|v| OPTS[:temp] = Float(v) }
opt.on('--hum=VAL'){|v| OPTS[:hum] = Float(v) }
opt.on('--bri=VAL'){|v| OPTS[:bri] = Integer(v) }
opt.parse!(ARGV)

# check argments

raise ArgumentError.new("--time=VAL required") unless OPTS.has_key?(:time)
raise ArgumentError.new("--temp=VAL required") unless OPTS.has_key?(:temp)
raise ArgumentError.new("--hum=VAL required") unless OPTS.has_key?(:hum)
raise ArgumentError.new("--bri=VAL required") unless OPTS.has_key?(:bri)

# record to DB
db = Sequel.connect("sqlite://#{DB_PATH}")
envs = db[:envs]
envs.insert(
  :time => OPTS[:time],
  :temperature => OPTS[:temp],
  :humidity => OPTS[:hum],
  :brightness => OPTS[:bri]
)
puts "Item count: #{envs.count}"

# output html

temp = OPTS[:temp].to_s
hum = OPTS[:hum].to_s

html = File.open(TEMPLATE_PATH){|f| f.read }
html.gsub!("$TEMP$", temp).gsub!("$HUM$", hum)
File.open(OUTPUT_PATH, "w"){|f| f.print html }

puts "HTML output OK => #{OUTPUT_PATH}"

