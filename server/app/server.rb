require 'sinatra'
require 'json'
require 'yaml'

BASIC_PATH = File.join(File.dirname(__FILE__) , "basic.txt")
PASSCODE_PATH = File.join(File.dirname(__FILE__) , "passcode.txt")
CMD_FILE_PATH = File.join(File.dirname(__FILE__) , "command.json")
DATA_PATH = File.join(File.dirname(__FILE__) , "latest.json")
TEMPLATE_PATH = File.join(File.dirname(__FILE__) , "template.html")
INDEX_PATH = File.join(settings.public_folder , "index.html")

set :port, 80
set :bind, "0.0.0.0"

BASIC_USER, BASIC_PASS = File.open(BASIC_PATH){|f| f.gets.chomp }.split(":",2)
PASSCODE = File.open(PASSCODE_PATH){|f| f.gets.chomp }

configure do
  unless File.exist? CMD_FILE_PATH
    File.open(CMD_FILE_PATH, "w"){|f| f.write({commands: [], lastid: -1}.to_json)}
  end
end

not_found do
  "404 Not Found"
end

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

get '/api/v1/envs' do
  begin
    str = File.open(DATA_PATH){|f| f.read }
    return str
  rescue
    return "{}"
  end
end

post '/api/v1/envs' do

  protected!

  time = params['time'].to_i
  tmp = params['temperature'].to_f
  hum = params['humidity'].to_f
  bri = params['brightness'].to_i

  data = {
    time: time,
    temperature: tmp,
    humidity: hum,
    brightness: bri
  }

  File.open(DATA_PATH, 'w'){|f| f.write(data.to_json) }

  html = File.open(TEMPLATE_PATH){|f|f.read}
  html.gsub!('$TEMP$', tmp.to_s)
  html.gsub!('$HUM$', hum.to_s)

  if bri > 10
    html.gsub!('$BODY_CLASS$', 'light')
  else
    html.gsub!('$BODY_CLASS$', 'dark')
  end

  File.open(INDEX_PATH, 'w'){|f| f.write html }

  headers 'Location' => "/api/v1/ac/commands/#{time}"
  status 201

  return data.to_json

end

post '/api/v1/ac/commands' do

  if params.has_key? 'passcode'
    unless PASSCODE == params['passcode']
      halt 401
    end
  else
    protected!
  end

  unless params.has_key? 'command'
    halt 400
  end

  newid = nil
  newcmd = nil

  File.open(CMD_FILE_PATH, 'r+') do |f|
    data = JSON.parse(f.read)
    newid = data['lastid'].to_i + 1
    data['lastid'] = newid
    newcmd = { id: newid, command: params['command'] }
    data['commands'].push newcmd
    f.rewind
    f.write data.to_json
    f.truncate(f.tell)
  end

  headers 'Location' => "/api/v1/ac/commands/#{newid}"
  status 201 

  return newcmd.to_json

end

delete '/api/v1/ac/commands/*' do |id|

  protected!

  unless id =~ /\d+/
    halt 400
  end

  File.open(CMD_FILE_PATH, 'r+') do |f|
    data = JSON.parse(f.read)
    data['commands'].reject!{|cmd|cmd['id'].to_s == id}
    puts data.to_json
    f.rewind
    f.write data.to_json
    f.truncate(f.tell)
  end

  status 200
  return ""

end

get '/api/v1/ac/commands' do
  return JSON.parse(File.open(CMD_FILE_PATH, 'r') {|f| f.read})['commands'].to_json
end

get '/' do
  send_file File.join(settings.public_folder, 'index.html')
end

get '/*' do |path|
  send_file File.join(settings.public_folder, path ,'index.html')
end
