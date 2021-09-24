################################################################
################################################################
################################################################
# Define source files
#
set (THREAD_FILES 
	UDPemitter.h
	UDPemitter.cpp
	tcp_communication.h
	tcp_communication.cpp
	rt32tmsrv.cpp
)


set (RT32TMSRV_HEADERS
TCPcommand.h
StringParser.h
)

set (RT32TMSRV_SOURCES 
TCPcommand.cpp
StringParser.cpp
)

################################################################
################################################################
################################################################
# Add library targets
#

add_library(rt32tmsrv_comm SHARED ${RT32TMSRV_SOURCES} )


################################################################
################################################################
################################################################
# Define list of libraries required for linking
#


set (LIBRARIES_ALL 
	boost_program_options
	boost_filesystem
	boost_system
	pthread
	rt32tmsrv_comm
	${yaml_LIB}
#	${exiv2_LIB}
#	OpenMP::OpenMP_CXX
	)


################################################################
################################################################
################################################################
# Add executable targets
#
add_executable(rt32tmsrv rt32tmsrv.cpp ${THREAD_FILES} )


################################################################
################################################################
################################################################

# Link targets

target_link_libraries(rt32tmsrv ${LIBRARIES_ALL} )

################################################################
################################################################
################################################################

# make docker image

add_custom_target(docker_image 
#	ALL
    COMMAND docker build . -t rt32tmsrv:${VERSION_STRING}
    COMMENT "Building docker image"
    DEPENDS rt32tmsrv
#    VERBATIM
    )
add_custom_target(docker_image_save
    docker save rt32tmsrv:${VERSION_STRING} -o rt32tmsrv-v${VERSION_STRING}.tar
    COMMENT "Saving docker image"
    DEPENDS docker_image
    )
  
add_custom_target(docker_image_send_galaxy
	scp rt32tmsrv-v${VERSION_STRING}.tar rt32time@galaxy:rt32tmsrv
#	scp config.txt rt32time@galaxy:rt32tmsrv
	COMMENT "Transferring docker image to galaxy"
	)
	