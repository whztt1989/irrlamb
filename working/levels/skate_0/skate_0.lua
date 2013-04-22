
-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 0 then
		Level.Win()
	end
end

-- Set up goal
GoalCount = 4

-- Set up level
Camera.SetYaw(-45)

-- Build block stack
tCrate = Level.GetTemplate("crate")

BaseCount = 3
Y = 0.5
while BaseCount >= 0 do
	for i = 0, BaseCount do
		Level.CreateObject("box", tCrate, -BaseCount / 2 + i, Y, 0)
	end
	BaseCount = BaseCount - 1
	Y = Y + 1
end
