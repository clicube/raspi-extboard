require 'sinatra'
require 'json'
require 'digest/sha1'

TEMPLATE_PATH = File.dirname(__FILE__) + "/template.html"
OUTPUT_PATH = File.dirname(__FILE__) + "/public/index.html"
SALT_PATH = File.dirname(__FILE__) + "/salt.txt"

set :port, 80
set :bind, "0.0.0.0"

salt = File.open(SALT_PATH){|f|f.gets}

def get_param(params, name)
  begin
    Float(params[name])
  rescue
    nil
  end
end

get '/update' do

  tmp = params['tmp'].to_f
  puts "tmp=#{tmp}"

  hum = params['hum'].to_f
  puts "hum=#{hum}"

  bri = params['bri'].to_i
  puts "bri=#{bri}"

  time = params['time'].to_i
  puts "time=#{time}"

  # check secret
  secret = params['secret']
  puts "secret=#{secret}"

  str = tmp.to_s + hum.to_s + bri.to_s + time.to_s + salt

  secret_gen = Digest::SHA1.hexdigest(str)
  halt 400 if secret != secret_gen

  html = File.open(TEMPLATE_PATH){|f|f.read}
  html.gsub!("$TEMP$", tmp.to_s)
  html.gsub!("$HUM$", hum.to_s)

  if bri > 15
    html.gsub!("$BODY_CLASS$", "light")
  else
    html.gsub!("$BODY_CLASS$", "dark")
  end

  File.open(OUTPUT_PATH, 'w'){|f| f.write html }

  return { tmp: tmp, hum: hum, bri: bri }.to_json

end

get '/' do
  send_file File.join(settings.public_folder, 'index.html')
end
