option(ZUC_BUILD_REMOTE_CLIENT_TEST "RemoteClient-Testwerkzeug mitbauen" ON)

add_subdirectory(local_client_test)
if(ZUC_BUILD_REMOTE_CLIENT_TEST)
    add_subdirectory(remote_client_test)
endif()
