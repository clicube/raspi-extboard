FROM ruby
ADD app/Gemfile /root/app/
RUN gem install bundler && \
    cd /root/app && \
    bundle install --system
ADD app /root/app/
EXPOSE 80
CMD ["ruby", "/root/app/server.rb"]
