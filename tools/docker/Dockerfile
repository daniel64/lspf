FROM arch-devel
LABEL maintainer="rvjansen@xs4all.nl"
WORKDIR /
RUN svn co https://svn.code.sf.net/p/oorexx/code-0/main/trunk oorexx-code-0
WORKDIR /oorexx-code-0
RUN cmake .
RUN make install
WORKDIR /
RUN git clone https://github.com/daniel64/lspf.git
ADD mkdirs.sh /lspf
WORKDIR /lspf
RUN chmod +x mkdirs.sh
RUN /lspf/mkdirs.sh
WORKDIR /lspf/src
# Copy the current directory contents into the container at /app
ADD copypaths.sh /lspf
RUN sed -i 's/#define ZSYSPATH        "\/home\/daniel\/lspf"/#define ZSYSPATH        "\/root\/.lspf"/g' lspf.h
RUN ./compsetup
RUN ./setup
RUN ./comp1
WORKDIR /lspf/src/Apps
RUN    ./compapps
WORKDIR /lspf/
RUN    chmod +x copypaths.sh
RUN    ./copypaths.sh

# Define environment variables
ENV HOME=/root
ENV LOGNAME=root
ENV SHELL=/usr/sbin/bash
ENV PATH=/lspf/src:$PATH

# Default command  when the container launches
CMD bash

