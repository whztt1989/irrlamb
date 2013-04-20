
-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 0 then
		Level.Win()
	end
end

-- Display lose message
function OnHitZone(HitType, Zone, Object)
	
	if Object == Player then
		GUI.TutorialText("Hit [".. KEY_RESET .. "] to restart.", 15)
		return 1
	end
	
	return 0
end

-- Set up goal
GoalCount = 1

-- Show text
GUI.TutorialText("Get near the top of the ball, then tap [" .. KEY_BACK .."] to roll the ball forward.", 10)
