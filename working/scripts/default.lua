-- This script is run by every level to set up common variables, functions, etc.
Player = Object.GetPointer("player")

-- Collision callback for an orb
function OnHitOrb(MainObject, OtherObject)
	if OtherObject == Player then
		Orb.Deactivate(MainObject, "OnOrbDeactivate", 2)
	end
end
