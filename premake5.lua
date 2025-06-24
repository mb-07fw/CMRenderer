newaction {
    trigger     = "clean",
    description = "Clean all project files, build files and intermediate files",
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
    description = "Clean only build files and intermediate files",
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

	project "CMEngine"
		kind "ConsoleApp"
        language "C++"
        cppdialect "C++20"

		targetname "CMEngine"
		targetdir "build/%{cfg.buildcfg}/out/CMEngine/"
		objdir "build/%{cfg.buildcfg}/int/CMEngine/"

		includedirs {
			"CMEngine/include/",
			"CMCommon/include/",
			"CMRenderer/include/",
			"vendor/CMDep_DearImGui/",
			"vendor/CMDep_yaml-cpp/include/"
		}

		files {
			"CMEngine/include/**.hpp",
			"CMEngine/include/**.h",
			"CMEngine/src/**.hpp",
			"CMEngine/src/**.h",
			"CMEngine/src/**.cpp",

			"**/*.hlsl"
		}

		libdirs {
			"build/%{cfg.buildcfg}/vendor/out/CMDep_yaml-cpp/",
			"build/%{cfg.buildcfg}/vendor/out/CMDep_DearImGui/",
			"build/%{cfg.buildcfg}/out/CMRenderer/",
			"build/%{cfg.buildcfg}/out/CMCommon/"
		}

		dependson {
			"CMCommon",
			"CMDep_DearImGui", 
			"CMDep_yaml-cpp"
		}

		links {
			"CMCommon",
			"CMDep_DearImGui",
			"CMDep_yaml-cpp",
			"CMCommon.lib",
			"CMDep_DearImGui.lib",
			"CMDep_yaml-cpp.lib"
		}

		-- YAML_CPP_STATIC_DEFINE heeds to be defined for any other project that uses it as a .lib
		defines { "YAML_CPP_STATIC_DEFINE", "CM_CONFIG=\"%{cfg.buildcfg}\"" }

		filter { "configurations:Debug" }
			defines { "CM_DEBUG" }
			symbols "On"
		
		filter { "configurations:Release" }
			defines { "CM_NDEBUG", "CM_RELEASE" }
			optimize "On"

		filter { "configurations:Dist" }
			defines { "CM_NDEBUG", "CM_DIST" }
			optimize "Full"
			symbols "Off"

		filter {}

		vpaths {
			["CMEngine/Source Files/Core"] = {
				"CMEngine/src/Core/CME_*.cpp",
				"CMEngine/src/Windows/CME_WN*.h"
			},

			["CMEngine/Source Files/Core"] = {
				"CMEngine/include/Windows/CME_WN*.h"
			},

			--| Windows specific filters ... |--
			["CMEngine/Source Files/Windows"]  = {
				"CMEngine/src/Windows/CME_WN*.cpp",
				"CMEngine/src/Windows/CME_WN*.h"
			},

			["CMEngine/Include Files/Windows"] = {
				"CMEngine/include/Windows/CME_WN*.h"
			},

			-- --| DirectX specific filters ... |--
			["CMEngine/Source Files/DirectX"]  = {
				"CMEngine/src/DirectX/CME_DX*.cpp",
				"CMEngine/src/DirectX/CME_DX*.h"
			},
			
			["CMEngine/Include Files/DirectX"] = {
				"CMEngine/include/DirectX/CME_DX*.h"
			},

			["CMEngine/Shaders/Pixel"] = { "**/*PS.hlsl" },
			["CMEngine/Shaders/Vertex"] = { "**/*VS.hlsl" }
		}

		-- local function table_append(dst, src)
		-- 	for _, v in ipairs(src) do
		-- 		table.insert(dst, v)
		-- 	end
		-- end
		-- 
		-- local function add_core_src_files_to_vpath()
		-- 	local core_files = {}
		-- 
		-- 	table_append(core_files, os.matchfiles("CMEngine/src/CME_*.h*"))
		-- 	table_append(core_files, os.matchfiles("CMEngine/src/CME_*.cpp"))
		-- 	table_append(core_files, os.matchfiles("CMEngine/src/CME_*.hpp"))
		-- 
		-- 	local filtered = {}
		-- 
		-- 	for _, path in ipairs(core_files) do
		-- 		local filename = path:match("[^/\\]+$")  -- Extract filename
		-- 		if not filename:match("^CME_DX") and not filename:match("^CME_WN") then
		-- 			table.insert(filtered, path)
		-- 		end
		-- 	end
		-- 
		-- 	vpaths {
		-- 		["CMEngine/Source Files/Core"] = filtered
		-- 	}
		-- end
		-- 
		-- local function add_core_include_files_to_vpath()
		-- 	local core_files = os.matchfiles("CMEngine/include/CME_*.h*")
		-- 
		-- 	table_append(core_files, os.matchfiles("CMEngine/include/CME_*.hpp*"))
		-- 
		-- 	local filtered = {}
		-- 
		-- 	for _, path in ipairs(core_files) do
		-- 		local filename = path:match("[^/\\]+$")  -- Extract filename
		-- 		if not filename:match("^CME_DX") and not filename:match("^CME_WN") then
		-- 			table.insert(filtered, path)
		-- 		end
		-- 	end
		-- 
		-- 	vpaths {
		-- 		["CMEngine/Include Files/Core"] = filtered
		-- 	};
		-- end
		-- 
		-- add_core_src_files_to_vpath()
		-- add_core_include_files_to_vpath()

		shadermodel "5.0"
		
		filter("files:**.hlsl")
			flags("ExcludeFromBuild")
			--shaderobjectfileoutput("%{cfg.targetdir}" .. "%{file.basename}" .. ".cso")
		
		filter("files:**_PS.hlsl")
			removeflags("ExcludeFromBuild")
			shadertype("Pixel")
		
		filter("files:**_VS.hlsl")
			removeflags("ExcludeFromBuild")
			shadertype("Vertex")
		
		filter {}
		
		-- Warnings as errors
		shaderoptions { "/WX" }

		filter "action:vs*"
			warnings "High"        -- /W4
			buildoptions { "/sdl" } -- /sdl security flag
		filter {}

	project "CMCommon"
		kind "StaticLib"
        -- staticruntime "On"
        language "C++"
        cppdialect "C++20"

		targetname "CMCommon"
		targetdir "build/%{cfg.buildcfg}/out/CMCommon/"
		objdir "build/%{cfg.buildcfg}/int/CMCommon/"

		includedirs { "CMCommon/include." }

		files {
			"CMCommon/include/**.hpp",
			"CMCommon/include/**.h",
			"CMCommon/src/**.hpp",
			"CMCommon/src/**.h",
			"CMCommon/src/**.cpp"
		}

		defines { "CM_CONFIG=\"%{cfg.buildcfg}\"" }

		filter { "configurations:Debug" }
			defines { "CM_DEBUG" }
			symbols "On"
		
		filter { "configurations:Release" }
			defines { "CM_NDEBUG" }
			optimize "On"

		filter { "configurations:Dist" }
			defines { "CM_NDEBUG", "DIST" }
			optimize "Full"
			symbols "Off"

		filter {}

		filter "action:vs*"
			warnings "High"        -- /W4
			buildoptions { "/sdl" } -- /sdl security flag
		filter {}

	project "CMDep_DearImGui"
		kind "StaticLib"
		-- staticruntime "On"
		language "C++"
		cppdialect "C++20"

		targetname "CMDep_DearImGui"
		targetdir "build/%{cfg.buildcfg}/vendor/out/CMDep_DearImGui/"
		objdir "build/%{cfg.buildcfg}/vendor/int/CMDep_DearImGui/"

		includedirs { "vendor/CMDep_DearImGui/imgui/" }

		files {
			"vendor/CMDep_DearImGui/**.hpp",
			"vendor/CMDep_DearImGui/**.h",
			"vendor/CMDep_DearImGui/**.cpp"
		}

		filter { "configurations:Debug" }
			defines { "CM_DEBUG" }
			symbols "On"
		
		filter { "configurations:Release" }
			defines { "CM_NDEBUG" }
			optimize "On"

		filter { "configurations:Dist" }
			defines { "CM_NDEBUG", "DIST" }
			optimize "Full"
			symbols "Off"

		filter {}

		filter "action:vs*"
			warnings "High"        -- /W4
			buildoptions { "/sdl" } -- /sdl security flag
		filter {}

	project "CMDep_yaml-cpp"
		kind "StaticLib"
		-- staticruntime "On"
		language "C++"
		cppdialect "C++20"

		targetname "CMDep_yaml-cpp"
		targetdir "build/%{cfg.buildcfg}/vendor/out/CMDep_yaml-cpp/"
		objdir "build/%{cfg.buildcfg}/vendor/int/CMDep_yaml-cpp/"

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

		filter { "configurations:Debug" }
			defines { "CM_DEBUG" }
			symbols "On"
		
		filter { "configurations:Release" }
			defines { "CM_NDEBUG" }
			optimize "On"

		filter { "configurations:Dist" }
			defines { "CM_NDEBUG", "DIST" }
			optimize "Full"
			symbols "Off"

		filter {}

		filter "action:vs*"
			warnings "High"        -- /W4
			buildoptions { "/sdl" } -- /sdl security flag
		filter {}