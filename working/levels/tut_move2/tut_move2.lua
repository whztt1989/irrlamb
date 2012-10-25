-- Display tutorial text
function ShowMoreText()
	GUI.TutorialText("Get some speed, then jump as you cross\nthe blue patch. Hit [" .. KEY_RESET .. "] to retry the level.", 25)
end

-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 0 then
		Level.Win()
	end
end

-- Set up goal
GoalCount = 1

-- Show text
GUI.TutorialText("Jumping is very important for climbing\nhills.", 5)
Timer.DelayedFunction("ShowMoreText", 6)