ARG DOCKER_BASE
FROM $DOCKER_BASE
ARG DEVICE

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get --no-install-recommends install -yq git cmake build-essential \
  libgl1-mesa-dev libsdl2-dev \
  libsdl2-image-dev libsdl2-ttf-dev libsdl2-gfx-dev libboost-all-dev \
  libdirectfb-dev libst-dev mesa-utils xvfb x11vnc \
  python3-pip

RUN python3 -m pip install --upgrade pip setuptools wheel
RUN python3 -m pip install --no-cache-dir psutil dm-sonnet==1.*
RUN python3 -m pip list | grep 'tensorflow ' || python3 -m pip install --no-cache-dir tensorflow==1.15.*

RUN python3 -m pip install --no-cache-dir git+https://github.com/openai/baselines.git@master
COPY . /gfootball
RUN cd /gfootball && python3 -m pip install .
WORKDIR '/gfootball'
