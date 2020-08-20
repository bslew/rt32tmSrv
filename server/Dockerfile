
#FROM ubuntu:18.04
#RUN apt install libgl1-mesa-glx
#RUN apt install libglvnd0

FROM fedora:26
#RUN dnf -y install python3-pip-9.0.3-2.fc26.noarch python3 python3-mysql.x86_64 
#RUN dnf -y clean all
#RUN dnf -y -v update --releasever=26
RUN dnf -y install libstdc++-7.3.1-2.fc26.x86_64 

# for command line parser and TCP/IP
RUN dnf -y install boost-system boost-program-options boost-filesystem

# for http queries
#RUN dnf -y install curlpp.x86_64
# curlpp-devel.x86_64 

# for yaml, but unufortunatelly not in this docker repo, so we have to copy manually
#RUN dnf -y install yaml-cpp 
#yaml-cpp-devel
#libglvnd-glx-1.0.0-1.fc26.i686 libglvnd-glx-1.0.0-1.fc26.x86_64 
#RUN dnf -y install python3-qt5-5.9.1-2.fc26.x86_64
#RUN dnf -y install python3-matplotlib-qt5-2.0.0-2.fc26.2.x86_64 python3-matplotlib-2.0.0-2.fc26.2.x86_64 python3-scipy-0.19.1-1.fc26.x86_64 python3-numpy-1.12.1-1.fc26.x86_64
#RUN dnf -y install xorg-x11-apps.x86_64
#libglvnd.x86_64

# Use an official Python runtime as a parent image
#FROM python:3.6-slim 

# Set the working directory to /app
WORKDIR /app

# Copy the current directory contents into the container at /app
RUN mkdir app
COPY rt32tmsrv config.txt rt32tmsrv-state.yaml librt32tmsrv_comm.so /app/
COPY libyaml-cpp.so.0.5.3 libyaml-cpp.so.0.5 /app/

# Install any needed packages specified in requirements.txt
#RUN pip3 install --trusted-host pypi.python.org -r requirements.txt


# Make port 80 available to the world outside this container
EXPOSE 33001/TCP 33033/UDP

# Define environment variable
#ENV NAME World
#ENV DISPLAY :0
ENV LD_LIBRARY_PATH="/app:${LD_LIBRARY_PATH}"

# Run app.py when the container launches
#CMD ["./rt32tmsrv", "-h", "app-backend"]
CMD ["./rt32tmsrv"]
