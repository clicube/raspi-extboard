FROM cubing/ruby:2.2.3
ADD app /root/app/Gemfile
RUN gem install bundler && \
    cd /root/app && \
    bundle install --system
ADD app /root/app/
EXPOSE 80
CMD ["ruby", "/root/app/server.rb"]
