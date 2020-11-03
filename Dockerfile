FROM ubuntu:20.04

ENV TZ=America/Denver
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get -y -o Acquire::Check-Valid-Until=false -o Acquire::Check-Date=false update
RUN apt-get -y -o Acquire::Check-Valid-Until=false -o Acquire::Check-Date=false upgrade
RUN apt-get install -y build-essential ca-certificates cmake cmake-curses-gui curl gnupg2 ntp ntpdate python3 python3-dev python3-pip procps
RUN pip3 install conan && conan config set general.revisions_enabled=True && conan config set general.parallel_download=8
RUN apt-get install -y curl g++ gcc autoconf automake bison libc6-dev libffi-dev libgdbm-dev libncurses5-dev libsqlite3-dev libtool libyaml-dev make pkg-config sqlite3 zlib1g-dev libgmp-dev libreadline-dev libssl-dev

SHELL [ "/bin/bash", "-l", "-c" ]
RUN gpg --keyserver hkp://pool.sks-keyservers.net --recv-keys 409B6B1796C275462A1703113804BB82D39DC0E3 7D2BAF1CF37B13E2069D6956105BD0E739499BDB
RUN curl -sSL https://get.rvm.io | bash -s
RUN source /etc/profile.d/rvm.sh && rvm install 2.5.5 && rvm --default use 2.5.5
RUN usermod -a -G rvm root
RUN echo "source /etc/profile.d/rvm.sh" >> /root/.bashrc

CMD ["/bin/bash"]