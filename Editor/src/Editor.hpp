#pragma once

#include "EngineCore.hpp"

/* TODO: Move resources directory to Editor. (Update macros)
 *
 * Cool Powershell commands I'm gonna forget later --
 * 
 * Get-ChildItem "_directory_", "_directory_" -Recurse -File
 * Get-ChildItem "_directory_", "_directory_" -Recurse -File | Group-Object Extension | Sort-Object Count -Descending | Format-Table Count, Name
 * Get-ChildItem "Editor", "LibEngineCore" -Recurse -Include *.cpp, *.hpp | Get-Content | Measure-Object -Line
 * 
 * Worshippable command sent from the heavens by ChatGPT :
	 Get-ChildItem "Editor","LibEngineCore" -Recurse -Include *.cpp,*.hpp | Group-Object { $_.Directory.Name } |
	 ForEach-Object { 
	 $lines = ($_.Group | Get-Content | Measure-Object -Line).Lines
	    [PSCustomObject]@{ Directory = $_.Name; Lines = $lines }
	} | Sort-Object Lines -Descending | Format-Table -AutoSize
*/

namespace CMEngine::Editor
{
	class Editor
	{
	public:
		Editor() noexcept;
		~Editor() noexcept;

		void Run() noexcept;
	private:
		EngineCore m_Core;
		Scene::SceneID m_EditorSceneID = {};
	};
}