PATH=/usr/local/bin:/usr/bin:/bin
date
cd `dirname $0`
bundle exec ruby cron.rb
