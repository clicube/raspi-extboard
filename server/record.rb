require 'bundler/setup'
require 'sequel'
require 'optparse'

DB_PATH = File.dirname(__FILE__) + "/db.sqlite3"
TEMPLATE_PATH = File.dirname(__FILE__) + "/template.html"
GRAPH_TEMPLATE_PATH = File.dirname(__FILE__) + "/graph_template.html"
OUTPUT_PATH = "/home/cubing/html/house/index.html"
GRAPH_OUTPUT_PATH = "/home/cubing/html/house/graph.html"

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
  :datetime => OPTS[:time],
  :temperature => OPTS[:temp],
  :humidity => OPTS[:hum],
  :brightness => OPTS[:bri]
)
puts "Item count: #{envs.count}"

# output graph

data_js = envs.order_by(Sequel.desc(:datetime)).limit(144).to_a.reverse.map{|i|
  tmp = i[:brightness] > 15
  i[:light] = tmp ? 100:0
  "[\'#{Time.at(i[:datetime]).strftime("%R")}\',#{i[:temperature]},#{i[:humidity]},#{i[:light]}]"
}.join(",")

html = File.open(GRAPH_TEMPLATE_PATH){|f| f.read }
html
  .gsub!("$DATA$", data_js)

File.open(GRAPH_OUTPUT_PATH, "w"){|f| f.print html }

puts "HTML output OK => #{GRAPH_OUTPUT_PATH}"

# output html

if OPTS[:bri] > 15
  bri_state = "light"
else
  bri_state = "dark"
end

html = File.open(TEMPLATE_PATH){|f| f.read }
html
  .gsub!("$BODY_CLASS$", bri_state)
  .gsub!("$TEMP$", OPTS[:temp].to_s)
  .gsub!("$HUM$", OPTS[:hum].to_s)

File.open(OUTPUT_PATH, "w"){|f| f.print html }

puts "HTML output OK => #{OUTPUT_PATH}"

