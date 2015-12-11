require 'sinatra'
require 'json'
require 'digest/sha1'

TEMPLATE_PATH = File.dirname(__FILE__) + "/template.html"
OUTPUT_PATH = File.dirname(__FILE__) + "/public/index.html"
BASIC_PATH = File.dirname(__FILE__) + "/basic.txt"

set :port, 80
set :bind, "0.0.0.0"

BASIC_USER, BASIC_PASS = File.open(BASIC_PATH){|f| f.gets.chomp }.split(":",2)

helpers do
  def protected!
    unless authorized?
      response['WWW-Authenticate'] = %(Basic realm="Restricted Area")
      throw(:halt, [401, "Not authorized\n"])
    end
  end

  def authorized?
    @auth ||=  Rack::Auth::Basic::Request.new(request.env)
    @auth.provided? && @auth.basic? && @auth.credentials && @auth.credentials == [BASIC_USER, BASIC_PASS]
  end
end

post '/api/v1/envs' do

  protected!

  tmp = params['temperature'].to_f
  hum = params['humidity'].to_f
  bri = params['brightness'].to_i
  time = params['time'].to_i

  html = File.open(TEMPLATE_PATH){|f|f.read}
  html.gsub!("$TEMP$", tmp.to_s)
  html.gsub!("$HUM$", hum.to_s)

  if bri > 15
    html.gsub!("$BODY_CLASS$", "light")
  else
    html.gsub!("$BODY_CLASS$", "dark")
  end

  File.open(OUTPUT_PATH, 'w'){|f| f.write html }

  return { tmperature: tmp, humidity: hum, brightness: bri, time: time }.to_json

end


get '/' do
  send_file File.join(settings.public_folder, 'index.html')
end
