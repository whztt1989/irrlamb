-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 0 then
		Level.Win()
	end
end

-- Set up level
tConstraint = Level.GetTemplate("constraint_z")
tLog = Level.GetTemplate("log")

oLog = Level.CreateObject("log0", tLog, 2, 1.25, 1.75, 90, 0, 0)
--Level.CreateConstraint("constraint0", tConstraint, oLog, 0)
Object.SetAngularVelocity(oLog, 0, 0, 1)

-- Set up goal
GoalCount = 1
