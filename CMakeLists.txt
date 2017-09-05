cmake_minimum_required(VERSION 3.0)

project(WinsockTut)

include_directories(
	src
)

add_definitions(
	-DNOMINMAX
	-DWIN32_LEAN_AND_MEAN
	-D_WIN32_WINNT=0x0600
	-D_CRT_SECURE_NO_WARNINGS
	-D_SCL_SECURE_NO_WARNINGS
	-D_WINSOCK_DEPRECATED_NO_WARNINGS
	-DFD_SETSIZE=1024
)
	
set(SERVER_SOURCE_FILES
	src/main.cpp
	src/common/Utils.h
	src/common/Utils.cpp
	src/common/Factory.h
	src/common/Factory.cpp
	src/common/Protocol.h
	src/common/ChatRoom.h
	src/common/ChatRoom.cpp
	
	src/select/SelectChatServer.cpp
	src/select/SelectChatServer.h
	
	src/async_event/AsyncEventChatServer.cpp
	src/async_event/AsyncEventChatServer.h
	
	src/async_select/AsyncSelectChatServer.cpp
	src/async_select/AsyncSelectChatServer.h
	
	src/overlapped/OverlapChatServer.cpp
	src/overlapped/OverlapChatServer.h
	
	src/complete_routine/ComRoutineChatServer.cpp
	src/complete_routine/ComRoutineChatServer.h	
	
	src/iocp/IOCPChatServer.cpp
	src/iocp/IOCPChatServer.h
)

set(CLIENT_SOURCE_FILES
	tests/cppclient/ChatClient.h
	tests/cppclient/ChatClient.cpp
	tests/cppclient/main.cpp
)

macro(SOURCE_GROUP_BY_DIR source_files)
	set(sgbd_cur_dir ${CMAKE_CURRENT_SOURCE_DIR})
	foreach(sgbd_file ${${source_files}})
		string(REGEX REPLACE ${sgbd_cur_dir}/\(.*\) \\1 sgbd_fpath ${sgbd_file})
		string(REGEX REPLACE "\(.*\)/.*" \\1 sgbd_group_name ${sgbd_fpath})
		string(COMPARE EQUAL ${sgbd_fpath} ${sgbd_group_name} sgbd_nogroup)
		string(REPLACE "/" "\\" sgbd_group_name ${sgbd_group_name})
		if(sgbd_nogroup)
			set(sgbd_group_name "\\")
		endif(sgbd_nogroup)
		source_group(${sgbd_group_name} FILES ${sgbd_file})
	endforeach(sgbd_file)
endmacro(SOURCE_GROUP_BY_DIR)

SOURCE_GROUP_BY_DIR(SERVER_SOURCE_FILES)
SOURCE_GROUP_BY_DIR(CLIENT_SOURCE_FILES)
	
add_executable(ChatServer ${SERVER_SOURCE_FILES})
target_link_libraries(ChatServer ws2_32 mswsock)

add_executable(ChatClient ${CLIENT_SOURCE_FILES})
target_link_libraries(ChatServer ws2_32 mswsock)
