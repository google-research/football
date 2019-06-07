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
RUN if [ $DEVICE = "cpu" ]; then pip3 install 'tensorflow<2.0'; fi
RUN cd /gfootball && pip3 install .[tf_$DEVICE] --process-dependency-links
WORKDIR '/gfootball'
