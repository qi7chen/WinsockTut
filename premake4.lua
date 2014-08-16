--
-- Premake4 build script (http://industriousone.com/premake/download)
--

solution 'WinsockExamples'
    configurations {'Debug', 'Release'}
    language 'C++'
    flags {'ExtraWarnings'}
    targetdir 'bin'

    configuration 'Debug'
        defines { 'DEBUG' }
        flags { 'Symbols' }

    configuration 'Release'
        defines { 'NDEBUG' }
        flags { 'Symbols', 'Optimize' }

    project 'Socket'
        location 'build'
        kind 'ConsoleApp'
        uuid "AB7D1C15-7A44-41a7-8864-230D8E345608"
        defines
        {
            'WIN32_LEAN_AND_MEAN',
            '_WIN32_WINNT=0x0501',
            'NOMINMAX',
        }
        files
        {
            'src/common/utility.h',
            'src/common/utility.c',
            'src/socket/*.h',
            'src/socket/*.c',
        }
        includedirs
        {
            'src',
        }

    project 'Select'
        location 'build'
        kind 'ConsoleApp'
        uuid "8701594A-72B8-4a6a-AEF3-6B41BBC33E65"
        defines
        {
            'WIN32_LEAN_AND_MEAN',
            '_WIN32_WINNT=0x0501',
            'NOMINMAX',
        }
        files
        {
            'src/common/*.h',
            'src/common/*.c',
            'src/select/*.h',
            'src/select/*.c',
        }
        includedirs
        {
            'src',
        }

    project 'AsyncSelect'
        location 'build'
        kind 'ConsoleApp'
        uuid "83CA7514-377C-4957-8399-9E407CFDD8DF"
        defines
        {
            'WIN32_LEAN_AND_MEAN',
            '_WIN32_WINNT=0x0501',
            'NOMINMAX',
        }
        files
        {
            'src/common/*.h',
            'src/common/*.c',
            'src/async_select/*.h',
            'src/async_select/*.c',
        }
        includedirs
        {
            'src',
        }

    project 'AsyncEvent'
        location 'build'
        kind 'ConsoleApp'
        uuid "DB1835D4-DA1F-4499-8F21-6E1913C2122E"
        defines
        {
            'WIN32_LEAN_AND_MEAN',
            '_WIN32_WINNT=0x0501',
            'NOMINMAX',
        }
        files
        {
            'src/common/*.h',
            'src/common/*.c',
            'src/async_event/*.h',
            'src/async_event/*.c',
        }
        includedirs
        {
            'src',
        }

    project 'CompleteRoutine'
        location 'build'
        kind 'ConsoleApp'
        uuid "729FA509-952E-41f2-A33C-1E061FADE78A"
        defines
        {
            'WIN32_LEAN_AND_MEAN',
            '_WIN32_WINNT=0x0501',
            'NOMINMAX',
        }
        files
        {
            'src/common/*.h',
            'src/common/*.c',
            'src/complete_routine/*.h',
            'src/complete_routine/*.c',
        }
        includedirs
        {
            'src',
        }

    project 'Overlapped'
        location 'build'
        kind 'ConsoleApp'
        uuid "18A51657-E2D5-49a0-B3C7-FE2BE55AD98A"
        defines
        {
            'WIN32_LEAN_AND_MEAN',
            '_WIN32_WINNT=0x0501',
            'NOMINMAX',
        }
        files
        {
            'src/common/*.h',
            'src/common/*.c',
            'src/overlapped/*.h',
            'src/overlapped/*.c',
        }
        includedirs
        {
            'src',
        }

    project 'IOCPServer'
        location 'build'
        kind 'ConsoleApp'
        uuid "588E072A-B1B8-4b36-B9BC-0E82547C7344"
        defines
        {
            'WIN32_LEAN_AND_MEAN',
            '_WIN32_WINNT=0x0501',
            'NOMINMAX',
        }
        files
        {
            'src/common/*.h',
            'src/common/*.c',
            'src/iocp_server/*.h',
            'src/iocp_server/*.c',
        }
        includedirs
        {
            'src',
        }
    project 'IOCPClient'
        location 'build'
        kind 'ConsoleApp'
        uuid "FC0C8C22-35CF-4ce4-9A90-F99AD63A7BEA"
        defines
        {
            'WIN32_LEAN_AND_MEAN',
            '_WIN32_WINNT=0x0501',
            'NOMINMAX',
        }
        files
        {
            'src/common/*.h',
            'src/common/*.c',
            'src/iocp_client/*.h',
            'src/iocp_client/*.c',
        }

        includedirs
        {
            'src',
        }

