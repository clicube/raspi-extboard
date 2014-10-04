# require 'sequel'
require 'optparse'

TEMPLATE_PATH = File.dirname(__FILE__) + "/template.html"
OUTPUT_PATH = "/home/cubing/html/house/index.html"

OPTS = {}
opt = OptionParser.new
opt.on('--time=VAL'){|v| OPTS[:time] = v }
opt.on('--temp=VAL'){|v| OPTS[:temp] = v.to_f }
opt.on('--hum=VAL'){|v| OPTS[:hum] = v.to_f }
opt.on('--bri=VAL'){|v| OPTS[:bri] = v.to_i }
opt.parse(ARGV)

temp = OPTS[:temp].to_s
hum = OPTS[:hum].to_s

html = File.open(TEMPLATE_PATH){|f| f.read }
html.gsub!("$TEMP$", temp).gsub!("$HUM$", hum)
File.open(OUTPUT_PATH, "w"){|f| f.print html }

puts "file output OK."
puts OUTPUT_PATH
