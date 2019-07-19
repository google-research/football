ARG DOCKER_BASE
FROM $DOCKER_BASE
ARG DEVICE

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -yq git cmake build-essential \
  libgl1-mesa-dev libsdl2-dev \
  libsdl2-image-dev libsdl2-ttf-dev libsdl2-gfx-dev libboost-all-dev \
  libdirectfb-dev libst-dev mesa-utils xvfb x11vnc libsqlite3-dev \
  glee-dev libsdl-sge-dev python3-pip

COPY . /gfootball
RUN cd /gfootball && pip3 install .[tf_$DEVICE]
RUN pip3 install git+https://github.com/openai/baselines.git@master
WORKDIR '/gfootball'
