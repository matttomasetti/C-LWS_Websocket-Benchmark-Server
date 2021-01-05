FROM ubuntu

# Set timezone
ENV TZ=America/New_York
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

# Add source files to docker image
ADD .	/home/websocket

# Update and install dependencies
RUN apt-get -y update \
    && apt-get -y upgrade \
    && apt-get -y install cmake git build-essential libssl-dev libjson-c-dev

#compile libwebsockets
RUN cd /home \
    && git clone https://github.com/warmcat/libwebsockets.git \
    && cd libwebsockets \
    && cmake . \
    && make \
    && make install

#compile json-c
RUN cd .. \
    && git clone https://github.com/json-c/json-c.git \
    && mkdir json-c-build \
    && cd json-c-build \
    && cmake ../json-c \
    && make \
    && make test \
    && make install

#compile websocket
RUN cd ../websocket \
    && cmake configure . \
    && cmake . \
    && make

EXPOSE 8080

WORKDIR /home/websocket
CMD ["./c-lws-websocket-server"]
