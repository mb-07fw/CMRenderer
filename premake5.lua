newaction {
    trigger     = "clean",
    description = "Clean all project files, build files and intermediate files.",
    execute     = function ()
        os.rmdir("bin")
        os.rmdir("build")

        os.rmdir(".vs")
        os.remove("**.sln")
        os.remove("**.vcxproj")
        os.remove("**.vcxproj.filters")
        os.remove("**.vcxproj.user")
        print("Full clean complete.")
    end
}

newaction {
    trigger     = "clean-build",
    description = "Removes the build directory.",
    execute     = function ()
        os.rmdir("bin")
        os.rmdir("build")
        print("Build clean complete.")
    end
}

newaction {
    trigger     = "clean-proj",
    description = "Clean only generated project files.",
    execute     = function ()
        os.rmdir(".vs")
        os.remove("**.sln")
        os.remove("**.vcxproj")
        os.remove("**.vcxproj.filters")
        os.remove("**.vcxproj.user")
        print("Project clean complete.")
    end
}

workspace "CMEngine"
	configurations { "Debug", "Release", "Dist" }
    startproject "CMEngine"
    architecture "x64"

	filter { "configurations:Debug" }
		defines { "CM_DEBUG" }
		symbols "On"
		
	filter { "configurations:Release" }
		defines { "CM_NDEBUG" }
		optimize "On"

	filter { "configurations:Dist" }
		defines { "CM_NDEBUG", "CM_DIST" }
		optimize "Full"
		symbols "Off"

	filter "action:vs*"
		warnings "High"        -- /W4
		buildoptions { "/sdl" } -- /sdl security flag

	filter {}

	shadermodel "5.0"
	shaderoptions { "/WX" } -- Warnings as errors

	defines { 'CM_CONFIG="%{cfg.buildcfg}"' }

	project "CMEngine"
		kind "ConsoleApp"
        language "C++"
        cppdialect "C++20"

		pchheader "Core/PCH.hpp"
		pchsource "CMEngine/src/Core/PCH.cpp"

		targetname "CMEngine"
		targetdir "build/%{cfg.buildcfg}/CMEngine/out/"
		objdir "build/%{cfg.buildcfg}/CMEngine/int/"

		includedirs {
			"CMEngine/include/",
			"vendor/CMDep_DearImGui/",
			"vendor/CMDep_yaml-cpp/include/"
		}

		files {
			"CMEngine/include/**.hpp",
			"CMEngine/include/**.h",
			"CMEngine/src/**.hpp",
			"CMEngine/src/**.h",
			"CMEngine/src/**.cpp",

			-- Collect all .hlsl files to filter them later.
			"**/*.hlsl"
		}

		libdirs {
			"build/%{cfg.buildcfg}/vendor/CMDep_yaml-cpp/out/",
			"build/%{cfg.buildcfg}/vendor/CMDep_DearImGui/out/",
		}

		dependson {
			"CMDep_DearImGui", 
			"CMDep_yaml-cpp"
		}

		links {
			"CMDep_DearImGui",
			"CMDep_yaml-cpp",
			"CMDep_DearImGui.lib",
			"CMDep_yaml-cpp.lib"
		}

		local CM_SOURCE_DIR = os.getcwd()

		-- Escape backslashes for C++ string literal compatibility
		local escapedSourceDir = CM_SOURCE_DIR:gsub("\\", "\\\\")

		-- Note to self: YAML_CPP_STATIC_DEFINE heeds to be defined for any other project that uses it as a .lib
		defines { "YAML_CPP_STATIC_DEFINE", 'CM_SOURCE_DIR="' .. escapedSourceDir .. '"' }

		vpaths {
			["CMEngine/Source Files/Common"] = {
				"CMEngine/src/Common/*.cpp",
				"CMEngine/src/Common/*.hpp"
			},

			["CMEngine/Include Files/Common"] = {
				"CMEngine/include/Common/*.hpp"
			},

			["CMEngine/Source Files/Core"] = {
				"CMEngine/src/Core/*.cpp",
				"CMEngine/src/Core/*.hpp"
			},

			["CMEngine/Include Files/Core"] = {
				"CMEngine/include/Core/*.hpp"
			},

			--| Windows specific filters ... |--
			["CMEngine/Source Files/Win"]  = {
				"CMEngine/src/Win/*.cpp",
				"CMEngine/src/Win/*.hpp"
			},

			["CMEngine/Include Files/Win"] = {
				"CMEngine/include/Win/*.hpp"
			},

			-- --| DirectX specific filters ... |--
			["CMEngine/Source Files/DX"]  = {
				"CMEngine/src/DX/*.cpp",
				"CMEngine/src/DX/*.hpp"
			},
			
			["CMEngine/Include Files/DirectX"] = {
				"CMEngine/include/DX/*.hpp"
			},

			["CMEngine/Shaders/Pixel"] = { "**/*PS.hlsl" },
			["CMEngine/Shaders/Vertex"] = { "**/*VS.hlsl" }
		}

		filter("files:**.hlsl")
			flags("ExcludeFromBuild")
		
		filter("files:**_PS.hlsl")
			removeflags("ExcludeFromBuild")
			shadertype("Pixel")
		
		filter("files:**_VS.hlsl")
			removeflags("ExcludeFromBuild")
			shadertype("Vertex")
		
		filter {}

	project "CMDep_DearImGui"
		kind "StaticLib"
		language "C++"
		cppdialect "C++20"

		targetname "CMDep_DearImGui"
		targetdir "build/CMDep_DearImGui/%{cfg.buildcfg}/out/"
		objdir "build/CMDep_DearImGui/%{cfg.buildcfg}/int/"

		includedirs { "vendor/CMDep_DearImGui/imgui/" }

		files {
			"vendor/CMDep_DearImGui/**.hpp",
			"vendor/CMDep_DearImGui/**.h",
			"vendor/CMDep_DearImGui/**.cpp"
		}

	project "CMDep_yaml-cpp"
		kind "StaticLib"
		language "C++"
		cppdialect "C++20"

		targetname "CMDep_yaml-cpp"
		targetdir "build/CMDep_yaml-cpp/%{cfg.buildcfg}/out/"
		objdir "build/CMDep_yaml-cpp/%{cfg.buildcfg}/int/"

		includedirs { "vendor/CMDep_yaml-cpp/include/" }

		-- Needs to be defined for any other project that uses it as a .lib
		defines { "YAML_CPP_STATIC_DEFINE" }

		files {
			"vendor/CMDep_yaml-cpp/include/yaml-cpp/**.hpp",
			"vendor/CMDep_yaml-cpp/include/yaml-cpp/**.h",
			"vendor/CMDep_yaml-cpp/src/**.hpp",
			"vendor/CMDep_yaml-cpp/src/**.h",
			"vendor/CMDep_yaml-cpp/src/**.cpp"
		}