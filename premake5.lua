--
-- Premake script (http://premake.github.io)
--

solution 'WinsockTut'
    configurations  {'Debug', 'Release'}
    language        'C'
    flags           'ExtraWarnings'
    targetdir       'bin'

    filter 'configurations:Debug'
        defines     { 'DEBUG' }
        flags       { 'Symbols' }

    filter 'configurations:Release'
        defines     { 'NDEBUG' }
        flags       { 'Symbols'}
        optimize    'On'

    filter 'action:vs*'
        defines
        {
            'WIN32',
            'NOMINMAX',
            'WIN32_LEAN_AND_MEAN',
            '_WIN32_WINNT=0x0600',
            '_CRT_SECURE_NO_WARNINGS',
            '_SCL_SECURE_NO_WARNINGS',
            '_WINSOCK_DEPRECATED_NO_WARNINGS',
        }
        includedirs 'src'
        links
        {
            'ws2_32',
            'mswsock',
        }        

    project 'Socket'
        targetname  'socket'
        location    'build'
        kind        'ConsoleApp'
        files
        {
            'src/common/*.h',
            'src/common/*.c',
            'src/socket/*.h',
            'src/socket/*.c',
        }

    project 'Select'
        targetname  'select'
        location    'build'
        kind        'ConsoleApp'
        defines     'FD_SETSIZE=1024'
        files
        {
            'src/common/*.h',
            'src/common/*.c',
            'src/select/*.h',
            'src/select/*.c',
        }

    project 'AsyncSelect'
        targetname  'async_select'
        location    'build'
        kind        'ConsoleApp'
        files
        {
            'src/common/*.h',
            'src/common/*.c',
            'src/async_select/*.h',
            'src/async_select/*.c',
        }

    project 'AsyncEvent'
        targetname  'async_event'
        location    'build'
        kind        'ConsoleApp'
        files
        {
            'src/common/*.h',
            'src/common/*.c',
            'src/async_event/*.h',
            'src/async_event/*.c',
        }

    project 'CompleteRoutine'
        targetname  'complete_routine'
        location    'build'
        kind        'ConsoleApp'
        files
        {
            'src/common/*.h',
            'src/common/*.c',
            'src/complete_routine/*.h',
            'src/complete_routine/*.c',
        }

    project 'Overlapped'
        targetname  'overlapped'
        location    'build'
        kind        'ConsoleApp'
        files
        {
            'src/common/*.h',
            'src/common/*.c',
            'src/overlapped/*.h',
            'src/overlapped/*.c',
        }

    project 'IOCPServer'
        targetname  'iocp_server'
        location    'build'
        kind        'ConsoleApp'
        files
        {
            'src/common/*.h',
            'src/common/*.c',
            'src/iocp_server/*.h',
            'src/iocp_server/*.c',
        }

    project 'IOCPClient'
        targetname  'iocp_client'
        location    'build'
        kind        'ConsoleApp'
        files
        {
            'src/common/*.h',
            'src/common/*.c',
            'src/iocp_client/*.h',
            'src/iocp_client/*.c',
        }

