-- Called when an orb is deactivated
function OnOrbDeactivate()
	GoalCount = GoalCount - 1
	if GoalCount == 0 then
		Level.Win()
	end
end

-- Display lose message
function OnHitZone(HitType, Zone, HitObject)
	
	if HitObject == Player then
		Level.Lose()
		return 1
	else
		Object.SetLifetime(HitObject, 2);
	end
	
	return 0
end

-- Set up level
Camera.SetYaw(25)

tLog = Level.GetTemplate("log")
tOrb = Level.GetTemplate("orb")

X = 0;
Z = -10;
oLog = Level.CreateObject("log0", tLog, X, 6, Z, 0, 0, 0);
X = X + 3;
Z = Z + 4;
oLog = Level.CreateObject("log1", tLog, X, 6, Z, 0, 0, 0);
X = X - 4;
Z = Z + 5;
oLog = Level.CreateObject("log2", tLog, X, 6, Z, 0, 0, 0);
X = X + 1;
Z = Z + 4;
oLog = Level.CreateObject("log3", tLog, X, 6, Z, 0, 0, 0);
X = X - 5;
Z = Z + 1;
oLog = Level.CreateObject("log4", tLog, X, 6, Z, 0, 0, 0);
Level.CreateObject("orb0", tOrb, X, 12, Z, 0, 0, 0);

-- Set up goal
GoalCount = 3
