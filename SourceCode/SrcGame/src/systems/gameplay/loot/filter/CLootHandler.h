#pragma once

#include <map>

struct TRANS_ITEM;

class CLootFilter
{
public:
	bool getEnabled() const { return bEnabled; }
	void setEnabled(bool value) { bEnabled = value; }

	bool getHpPot() const { return bHpPot; }
	void setHpPot(bool value) { bHpPot = value; }

	bool getMpPot() const { return bMpPot; }
	void setMpPot(bool value) { bMpPot = value; }

	bool getSpPot() const { return bSpPot; }
	void setSpPot(bool value) { bSpPot = value; }

	bool getGold() const { return bGold; }
	void setGold(bool value) { bGold = value; }

	bool getAmulets() const { return bAmulets; }
	void setAmulets(bool value) { bAmulets = value; }

	bool getRings() const { return bRings; }
	void setRings(bool value) { bRings = value; }

	bool getSheltoms() const { return bSheltoms; }
	void setSheltoms(bool value) { bSheltoms = value; }

	bool getForce() const { return bForce; }
	void setForce(bool value) { bForce = value; }

	bool getPremiums() const { return bPremiums; }
	void setPremiums(bool value) { bPremiums = value; }

	bool getCrystals() const { return bCrystals; }
	void setCrystals(bool value) { bCrystals = value; }

	bool getDefense() const { return bDefense; }
	void setDefense(bool value) { bDefense = value; }

	bool getOffense() const { return bOffense; }
	void setOffense(bool value) { bOffense = value; }

	bool getEverything() const { return bEverything; }
	void setEverything(bool value) { bEverything = value; }

	bool getNonSpec() const { return bNonSpec; }
	void setNonSpec(bool value) { bNonSpec = value; }

	// Generic class-specialization preference API.  Legacy named accessors remain
	// as a compatibility facade for the current settings window/config file.
	bool getClassSpec(int legacyJobCode) const
	{
		std::map<int, bool>::const_iterator it = classSpecs.find(legacyJobCode);
		return it != classSpecs.end() && it->second;
	}
	void setClassSpec(int legacyJobCode, bool value) { classSpecs[legacyJobCode] = value; }

	bool getFighter() const { return getClassSpec(1); }
	void setFighter(bool value) { setClassSpec(1, value); }
	bool getMech() const { return getClassSpec(2); }
	void setMech(bool value) { setClassSpec(2, value); }
	bool getArcher() const { return getClassSpec(3); }
	void setArcher(bool value) { setClassSpec(3, value); }
	bool getPikeman() const { return getClassSpec(4); }
	void setPikeman(bool value) { setClassSpec(4, value); }
	bool getAtalanta() const { return getClassSpec(5); }
	void setAtalanta(bool value) { setClassSpec(5, value); }
	bool getKnight() const { return getClassSpec(6); }
	void setKnight(bool value) { setClassSpec(6, value); }
	bool getMagician() const { return getClassSpec(7); }
	void setMagician(bool value) { setClassSpec(7, value); }
	bool getPriestess() const { return getClassSpec(8); }
	void setPriestess(bool value) { setClassSpec(8, value); }

	bool Filter(TRANS_ITEM* pItem);

private:
	bool bEnabled;
	bool bHpPot;
	bool bMpPot;
	bool bSpPot;
	bool bGold;
	bool bAmulets;
	bool bRings;
	bool bSheltoms;
	bool bForce;
	bool bPremiums;
	bool bCrystals;
	bool bDefense;
	bool bOffense;
	bool bEverything;
	bool bNonSpec;
	std::map<int, bool> classSpecs;
};

extern CLootFilter cLootFilter;



