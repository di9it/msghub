#LINK_LIBRARIES(msghub)

ADD_EXECUTABLE(server server.cpp)
TARGET_LINK_LIBRARIES(server msghub)

ADD_EXECUTABLE(client client.cpp)
TARGET_LINK_LIBRARIES(client msghub)
