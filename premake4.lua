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
            'src/common/*.h',
            'src/common/*.cpp',
            'src/socket/*.h',
            'src/socket/*.cpp',
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
            'src/common/*.cpp',
            'src/select/*.h',
            'src/select/*.cpp',
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
            'src/common/*.cpp',
            'src/async_select/*.h',
            'src/async_select/*.cpp',
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
            'src/common/*.cpp',
            'src/async_event/*.h',
            'src/async_event/*.cpp',
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
            'src/common/*.cpp',
            'src/complete_routine/*.h',
            'src/complete_routine/*.cpp',
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
            'src/common/*.cpp',
            'src/overlapped/*.h',
            'src/overlapped/*.cpp',
        } 
        includedirs
        {
            'src',
        }        
        
    project 'IOCP'
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
            'src/common/*.cpp',
            'src/iocp/*.h',
            'src/iocp/*.cpp',
        }
        includedirs
        {
            'src',
        }
        
solution 'TestClient'
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
    
    project 'TestClient'
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
            'src/common/*.cpp',
            'tests/test_client/*.h',
            'tests/test_client/*.cpp',
        }
        
        includedirs 
        {
            'src',
        }
        
        