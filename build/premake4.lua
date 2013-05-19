--
-- Premake4 build script (http://industriousone.com/premake/download)
--

function setTargetObjDir(outDir)
	for _, cfg in ipairs(configurations()) do
		for _, plat in ipairs(platforms()) do
			local action = _ACTION or ""
			local prj = project()
			local suffix = "_" .. cfg .. "_" .. plat .. "_" .. action
			targetPath = outDir
			suffix = string.lower(suffix)
			local obj_path = "../intermediate/" .. cfg .. "/" .. action .. "/" .. prj.name
			obj_path = string.lower(obj_path)
			configuration {cfg, plat}
				targetdir(targetPath)
				objdir(obj_path)
				targetsuffix(suffix)
		end
	end
end

solution 'winsock-samples'
    configurations {'Debug', 'Release'}
    location ('./' .. (_ACTION or ''))
    language 'C++'
    flags {'ExtraWarnings'}
    
    configuration 'Debug'
        defines { 'DEBUG' }
        flags { 'Symbols' }
        
    configuration 'Release'
        defines { 'NDEBUG' }
        flags { 'Optimize' }
    
    configuration "vs*"
        defines { "_CRT_SECURE_NO_WARNINGS" }
    
    project 'socket'
        kind 'ConsoleApp'
        
        files {
            '../src/common/cmndef.h',
            '../src/common/utility.h',
            '../src/common/utility.cpp',
            '../src/socket/**.cpp'
        }
        setTargetObjDir('bin')
        
    project 'select'
        kind 'ConsoleApp'
        
        files {
            '../src/common/cmndef.h',
            '../src/common/utility.h',
            '../src/common/utility.cpp',
            '../src/select/**.cpp'
        }
        setTargetObjDir('bin')

    project 'async_select'
        kind 'ConsoleApp'
        
        files {
            '../src/common/cmndef.h',
            '../src/common/utility.h',
            '../src/common/utility.cpp',
            '../src/async_select/**.h',
            '../src/async_select/**.cpp'
        }
        setTargetObjDir('bin')

    project 'async_event'
        kind 'ConsoleApp'
        
        files {
            '../src/common/cmndef.h',
            '../src/common/mutex.h',
            '../src/common/utility.h',
            '../src/common/utility.cpp',
            '../src/async_event/**.h',
            '../src/async_event/**.cpp'
        }
        setTargetObjDir('bin')

    project 'complete_routine'
        kind 'ConsoleApp'
        
        files {
            '../src/common/cmndef.h',
            '../src/common/utility.h',
            '../src/common/utility.cpp',
            '../src/complete_routine/**.h',
            '../src/complete_routine/**.cpp'
        }
        setTargetObjDir('bin')

    project 'overlapped'
        kind 'ConsoleApp'
        
        files {
            '../src/common/cmndef.h',
            '../src/common/utility.h',
            '../src/common/utility.cpp',
            '../src/overlapped/**.h',
            '../src/overlapped/**.cpp'
        } 
        setTargetObjDir('bin')
        
    project 'iocp'
        kind 'ConsoleApp'
        
        files {
            '../src/common/cmndef.h',
            '../src/common/mutex.h',
            '../src/common/utility.h',
            '../src/common/utility.cpp',
            '../src/iocp/**.h',
            '../src/iocp/**.cpp'
        }
        setTargetObjDir('bin')
        