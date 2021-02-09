ADD_DEFINITIONS(-DBOOST_TEST_DYN_LINK)
ADD_EXECUTABLE(tester
    client_onserverfailure.cpp
    connect.cpp
    create.cpp
    emptymsg.cpp
    main.cpp
    server_onclientfailure.cpp
    subscribe.cpp
    toobig.cpp
    unsubscribe.cpp
)
TARGET_LINK_LIBRARIES(tester msghub)
TARGET_LINK_LIBRARIES(tester ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
TARGET_LINK_LIBRARIES(tester ${Boost_TEST_EXEC_MONITOR_LIBRARY})

add_test(NAME tester COMMAND tester)
