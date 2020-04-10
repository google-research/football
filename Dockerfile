ARG DOCKER_BASE
FROM nvidia/cuda:10.0-cudnn7-devel-ubuntu18.04


ARG DEVICE
ENV NVIDIA_DRIVER_CAPABILITIES ${NVIDIA_DRIVER_CAPABILITIES},display
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -yq git cmake build-essential \
  libgl1-mesa-dev libsdl2-dev \
  libsdl2-image-dev libsdl2-ttf-dev libsdl2-gfx-dev libboost-all-dev \
  libdirectfb-dev libst-dev mesa-utils xvfb x11vnc \
  libsdl-sge-dev python3-pip


RUN apt-get update && apt-get install -y --no-install-recommends \
        mesa-utils xauth && \
    rm -rf /var/lib/apt/lists/* && \
    apt-get update && apt-get install -y \
    curl \
    ca-certificates \
    sudo \
    vim \
    git \
    bzip2 \
    libx11-6 && \
    rm -rf /var/lib/apt/lists/*

RUN python3 -m pip install --upgrade pip
RUN pip3 install tensorflow-gpu==1.15rc2 dm-sonnet psutil
RUN pip3 install git+https://github.com/openai/baselines.git@master

#COPY . /gfootball
#RUN cd /gfootball && pip3 install . -t /app 
#
#RUN apt-get update -y && \
# apt-get install software-properties-common -y && \
# add-apt-repository ppa:webupd8team/atom -y && \
# apt-get update -y && \
# apt-get install atom -y && \
# apm install remote-atom && \
# curl -o /usr/local/bin/rmate https://raw.githubusercontent.com/aurora/rmate/master/rmate && \
# sudo chmod +x /usr/local/bin/rmate && \
# mv /usr/local/bin/rmate /usr/local/bin/ratom && \
# apt-get install eog -y && \
# apt-get update -y && \
# pip install torchtext tqdm imageio configparser matplotlib tifffile pyquaternion panda3d==1.10.0 && \
# apt-get install -y x11-apps && \
# touch /root/.Xauthority
#
# CMD ["python3"]
# WORKDIR /app
