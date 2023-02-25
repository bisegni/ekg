cmake_minimum_required(VERSION 3.2)
project(epics-gateway)
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  SET(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/local CACHE PATH "default install directory" FORCE)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

include(ExternalProject)

set(CMAKE_CXX_STANDARD 20)
set(BOOST_VERSION 1.77.0)
set(EPICS_VERSION R7.0.7)
set(PROTOBUF_VERSION v22.0)
set(LIBLZ4_VERSION v1.9.2)
set(LIBRDKAFKA_VERSION v2.0.2)
set(SQLITE_VERSION master)
set(SQLITEORM_VERSION v1.8.1)
set(GTEST_VERSION 1.8.0)
set(CMAKE_BINARY_DIR ${CMAKE_INSTALL_PREFIX}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_INSTALL_PREFIX}/lib)
set(CMAKE_BINARY_OUTPUT_DIRECTORY ${CMAKE_INSTALL_PREFIX}/bin)

set(COMMON_SOURCE_FILES
  src/gateway/Gateway.cpp
  src/gateway/common/ProgramOptions.cpp  

  src/gateway/service/epics/EpicsChannel.cpp
  src/gateway/service/epics/EpicsChannelMonitor.cpp

  src/gateway/service/pubsub/IPublisher.cpp
  src/gateway/service/pubsub/ISubscriber.cpp

  src/gateway/service/pubsub/impl/kafka/RDKafkaBase.cpp
  src/gateway/service/pubsub/impl/kafka/RDKafkaPublisher.cpp
  src/gateway/service/pubsub/impl/kafka/RDKafkaSubscriber.cpp

  src/gateway/service/log/impl/BoostLogger.cpp
  )

set(SOURCE_FILES
    src/main.cpp
    ${COMMON_SOURCE_FILES}
  )

ADD_EXECUTABLE(
    ${PROJECT_NAME}
    ${SOURCE_FILES}
    )

set_target_properties(${PROJECT_NAME}
    PROPERTIES
    PUBLIC_HEADER "${EPICS_GATEWAY_INCLUDE}"
)
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        )

TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC src)
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${CMAKE_INSTALL_PREFIX}/include)
TARGET_LINK_DIRECTORIES(${PROJECT_NAME} PUBLIC ${CMAKE_INSTALL_PREFIX}/lib)

#-------------------------- Tests ---------------------
set(TEST_SOURCE_FILES
    test/test.cpp
    test/Gateway.cpp
    test/epics/epics.cpp
    test/pubsub/kafka.cpp
    test/common/ProgramOptions.cpp
    ${COMMON_SOURCE_FILES}
    )

ADD_EXECUTABLE(
    ${PROJECT_NAME}-test
    ${TEST_SOURCE_FILES}
)

add_test(
    NAME UnitTest
    COMMAND ${PROJECT_NAME}-test
)

install(TARGETS ${PROJECT_NAME}-test
        RUNTIME DESTINATION bin
        )
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME}-test PUBLIC src)
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME}-test PUBLIC ${CMAKE_INSTALL_PREFIX}/include)
TARGET_LINK_DIRECTORIES(${PROJECT_NAME}-test PUBLIC ${CMAKE_INSTALL_PREFIX}/lib)

#--------------------boost--------------------------
MESSAGE(STATUS "Configure boost version ${BOOST_VERSION}")

if (WIN32)
     set (BOOST_BOOTSTRAP_CMD ${BOOST_GIT_REPO_DIR}/bootstrap.bat)
     # It was bjam at one point; it is now b2
    set (BOOST_BUILD_CMD ${BOOST_GIT_REPO_DIR}/b2.exe)
elseif (UNIX)
    set (BOOST_BOOTSTRAP_CMD ./${BOOST_GIT_REPO_DIR}/bootstrap.sh)
    set (BOOST_BUILD_CMD ./${BOOST_GIT_REPO_DIR}/b2)
endif ()

ExternalProject_Add (
    boost
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/boost
    GIT_REPOSITORY https://github.com/boostorg/boost.git
    GIT_TAG boost-${BOOST_VERSION}
    GIT_PROGRESS 1
    BUILD_IN_SOURCE 1
    GIT_SUBMODULES_RECURSE true
    UPDATE_COMMAND ${BOOST_BOOTSTRAP_CMD}
    BUILD_COMMAND ${BOOST_BUILD_CMD} --with-system --with-program_options --with-log link=shared install --prefix=${CMAKE_INSTALL_PREFIX}
    INSTALL_COMMAND ""
    LOG_DOWNLOAD 1
    LOG_UPDATE 1
    LOG_CONFIGURE 1
    LOG_BUILD 1
    LOG_INSTALL 1
    )

SET(BUILD_LIBRARY ${BUILD_LIBRARY} boost_program_options boost_log boost_log_setup boost_thread)
ADD_DEPENDENCIES(${PROJECT_NAME} boost)
ADD_DEPENDENCIES(${PROJECT_NAME}-test boost)

#--------------------epics--------------------------
MESSAGE(STATUS "Configure EPICS version ${EPICS_VERSION}")
ExternalProject_Add(
 	epics
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/epics
 	GIT_REPOSITORY https://github.com/epics-base/epics-base.git
  GIT_TAG ${EPICS_VERSION}
  GIT_PROGRESS 1
  BUILD_IN_SOURCE 1
  #PATCH_COMMAND ${PROJECT_SOURCE_DIR}/tools/epics_patch.sh ${CMAKE_CURRENT_BINARY_DIR}
  CONFIGURE_COMMAND ""
	UPDATE_COMMAND    ""
	INSTALL_COMMAND   ""
    BUILD_COMMAND make -j 3  INSTALL_LOCATION=${CMAKE_INSTALL_PREFIX} install
  )

SET(BUILD_LIBRARY ${BUILD_LIBRARY} pvaClient pvAccess Com pvData pvDatabase pvAccessCA)
ADD_DEPENDENCIES(${PROJECT_NAME} epics)
ADD_DEPENDENCIES(${PROJECT_NAME}-test epics)
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${CMAKE_INSTALL_PREFIX}/include/os/${CMAKE_SYSTEM_NAME})
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${CMAKE_INSTALL_PREFIX}/include/pv)
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${CMAKE_INSTALL_PREFIX}/include/pva)
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME}-test PUBLIC ${CMAKE_INSTALL_PREFIX}/include/os/${CMAKE_SYSTEM_NAME})
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME}-test PUBLIC ${CMAKE_INSTALL_PREFIX}/include/pv)
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME}-test PUBLIC ${CMAKE_INSTALL_PREFIX}/include/pva)
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    MESSAGE(STATUS "Configure EPICS include for clang compiler")
    TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${CMAKE_INSTALL_PREFIX}/include/compiler/clang)
    TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME}-test PUBLIC ${CMAKE_INSTALL_PREFIX}/include/compiler/clang)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # using GCC
  MESSAGE(STATUS "Configure EPICS include for gcc compiler")
  TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${CMAKE_INSTALL_PREFIX}/include/compiler/gcc)
  TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME}-test PUBLIC ${CMAKE_INSTALL_PREFIX}/include/compiler/gcc)
endif()
TARGET_LINK_DIRECTORIES(${PROJECT_NAME} PUBLIC ${CMAKE_INSTALL_PREFIX}/lib/linux-x86_64)
TARGET_LINK_DIRECTORIES(${PROJECT_NAME}-test PUBLIC ${CMAKE_INSTALL_PREFIX}/lib/linux-x86_64)

#--------------------protobuf--------------------------
MESSAGE(STATUS "Configure protobuf version ${PROTOBUF_VERSION}")
ExternalProject_Add(
    protobuf
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/protobuf
 	  GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    GIT_TAG ${PROTOBUF_VERSION}
    GIT_PROGRESS 1
    LOG_DOWNLOAD 1
    LOG_UPDATE 1
    LOG_CONFIGURE 1
    LOG_BUILD 1
    LOG_INSTALL 1
    CMAKE_ARGS 
      -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
  )
SET(BUILD_LIBRARY ${BUILD_LIBRARY} )
ADD_DEPENDENCIES(${PROJECT_NAME} protobuf)
ADD_DEPENDENCIES(${PROJECT_NAME}-test protobuf)
#------------Nuraft----------------
MESSAGE(STATUS "Configure NuRAFT")
ExternalProject_Add(
  nuraft
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/nuraft
 	  GIT_REPOSITORY https://github.com/bisegni/NuRaft.git
    GIT_TAG feature/asio-module
    CMAKE_ARGS 
      -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
  )
SET(BUILD_LIBRARY ${BUILD_LIBRARY} )
ADD_DEPENDENCIES(${PROJECT_NAME} nuraft)
ADD_DEPENDENCIES(${PROJECT_NAME}-test nuraft)

#----------------kafka------------------
MESSAGE(STATUS "Configure liblz4 version ${LIBLZ4_VERSION}")
ExternalProject_Add(
  liblz4
  GIT_REPOSITORY https://github.com/lz4/lz4.git
  GIT_TAG ${LIBLZ4_VERSION}
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/liblz4
  SOURCE_SUBDIR contrib/cmake_unofficial
  CMAKE_ARGS 
    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_BUILD_TYPE=Release
    -DBUILD_STATIC_LIBS=OFF
    -DBUILD_SHARED_LIBS=ON
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON 
  #  -DCMAKE_CXX_STANDARD=11
  #  -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER} 
  #  -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
  LOG_DOWNLOAD ON
  LOG_CONFIGURE ON
  LOG_BUILD ON
  )
  
MESSAGE(STATUS "Configure librdkafka version ${LIBRDKAFKA_VERSION}")
ExternalProject_Add(
  librdkafa
  GIT_TAG ${LIBRDKAFKA_VERSION}
  GIT_REPOSITORY https://github.com/edenhill/librdkafka.git
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/librdkafa
  CMAKE_ARGS
    -DWITH_SSL=OFF 
    -DRDKAFKA_BUILD_EXAMPLES=OFF 
    -DRDKAFKA_BUILD_STATIC=OFF 
    -DRDKAFKA_BUILD_TESTS=OFF 
    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}  
    -DCMAKE_BUILD_TYPE=Release 
    # -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
    # -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
  LOG_DOWNLOAD 1
  LOG_UPDATE 1
  LOG_CONFIGURE 1
  LOG_BUILD 1
  LOG_INSTALL 1
)
SET(BUILD_LIBRARY ${BUILD_LIBRARY} rdkafka rdkafka++)
ADD_DEPENDENCIES(librdkafa liblz4)
ADD_DEPENDENCIES(${PROJECT_NAME} librdkafa liblz4)
ADD_DEPENDENCIES(${PROJECT_NAME}-test librdkafa liblz4)

#----------sqlite orm-----
MESSAGE(STATUS "Configure sqlite version ${SQLITE_VERSION}")
ExternalProject_Add(
  sqlite_dist
  GIT_TAG ${SQLITE_VERSION}
  GIT_REPOSITORY https://github.com/azadkuh/sqlite-amalgamation.git
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/sqlite
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}  
    -DCMAKE_BUILD_TYPE=Release 
  LOG_DOWNLOAD 1
  LOG_UPDATE 1
  LOG_CONFIGURE 1
  LOG_BUILD 1
  LOG_INSTALL 1
)

MESSAGE(STATUS "Configure sqlite orm version ${SQLITEORM_VERSION}")
ExternalProject_Add(
  sqlite_orm
  GIT_TAG ${SQLITEORM_VERSION}
  GIT_REPOSITORY https://github.com/fnc12/sqlite_orm.git
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/sqlite_orm
  CMAKE_ARGS
    -DSQLITE_ORM_ENABLE_CXX_20=ON
    -DBUILD_TESTING=OFF
    -DBUILD_EXAMPLES=OFF
    -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}  
    -DCMAKE_BUILD_TYPE=Release 
  LOG_DOWNLOAD 1
  LOG_UPDATE 1
  LOG_CONFIGURE 1
  LOG_BUILD 1
  LOG_INSTALL 1
)
ADD_DEPENDENCIES(sqlite_orm sqlite_dist)
SET(BUILD_LIBRARY ${BUILD_LIBRARY} sqlite3)
ADD_DEPENDENCIES(${PROJECT_NAME}  sqlite_orm sqlite_dist)
ADD_DEPENDENCIES(${PROJECT_NAME}-test sqlite_orm sqlite_dist)


#--------Google Test------
message (STATUS "Setup GTest version ${GTEST_VERSION}")
ExternalProject_Add(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
	  GIT_TAG release-${GTEST_VERSION}
    PREFIX "${CMAKE_CURRENT_BINARY_DIR}/gtest"
    CMAKE_ARGS
      -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON
)

ADD_DEPENDENCIES(${PROJECT_NAME}-test googletest)

ADD_DEFINITIONS(-DBOOST_LOG_DYN_LINK)
ADD_DEFINITIONS(-DBOOST_ALL_DYN_LINK)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC ${BUILD_LIBRARY} pthread)
TARGET_LINK_LIBRARIES(${PROJECT_NAME}-test PUBLIC ${BUILD_LIBRARY} gtest)
MESSAGE(STATUS "Linked library: ${BUILD_LIBRARY}")
MESSAGE(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
# enable_testing()
# add_test(NAME    unit_test 
#          COMMAND unit_test)