FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

COPY gfootball /tmp/gfootball

RUN apt-get update && \
      apt-get --no-install-recommends install -yq \
      git cmake build-essential \
      libgl1-mesa-dev libsdl2-dev \
      libsdl2-image-dev libsdl2-ttf-dev libsdl2-gfx-dev libboost-all-dev \
      libdirectfb-dev libst-dev mesa-utils xvfb x11vnc \
      libsdl-sge-dev python3-pip && \
      python3 -m pip install /tmp/gfootball && \
      rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*
