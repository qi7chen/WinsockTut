--
-- Premake script (http://premake.github.io)
--

solution 'WinsockTut'
    configurations  {'Debug', 'Release'}
    targetdir       'bin'
    language 'C++'

    filter 'configurations:Debug'
        defines 'DEBUG'
        symbols 'On'
        

    filter 'configurations:Release'
        defines 'NDEBUG'
        symbols 'On'
        flags 'Optimize'

    filter 'action:vs*'
        defines
        {
            'WIN32',
            'WIN32_LEAN_AND_MEAN',
            '_WIN32_WINNT=0x0600',
            '_CRT_SECURE_NO_WARNINGS',
            'NOMINMAX',
        }
        links 'ws2_32'


	project 'libWinNet'
        location 'build'
        kind 'StaticLib'

        files
        {
            'src/**.cpp',
            'src/**.h',
        }
        includedirs 'src'


    project 'ExampleEcho'
        location    'build'
        kind        'ConsoleApp' 
        files
        {
            'examples/echo/*.h',
            'examples/echo/*.cpp',
        }
        includedirs 'src'
        links 'libWinNet'


            