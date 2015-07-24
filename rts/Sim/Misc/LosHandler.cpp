/* This file is part of the Spring engine (GPL v2 or later), see LICENSE.html */


#include "LosHandler.h"
#include "ModInfo.h"

#include "Sim/Units/Unit.h"
#include "Sim/Misc/TeamHandler.h"
#include "Map/ReadMap.h"
#include "System/Log/ILog.h"
#include "System/TimeProfiler.h"
#include "System/creg/STL_Deque.h"


CR_BIND(LosInstance, )
CR_BIND(CLosHandler, )
CR_BIND(CLosHandler::DelayedInstance, )

CR_REG_METADATA(LosInstance,(
	CR_IGNORED(losSquares),
	CR_MEMBER(losSize),
	CR_MEMBER(airLosSize),
	CR_MEMBER(refCount),
	CR_MEMBER(allyteam),
	CR_MEMBER(basePos),
	CR_MEMBER(baseSquare),
	CR_MEMBER(baseAirPos),
	CR_MEMBER(hashNum),
	CR_MEMBER(baseHeight),
	CR_MEMBER(toBeDeleted)
))

void CLosHandler::PostLoad()
{
	for (auto& bucket: instanceHash) {
		for (LosInstance* li: bucket) {
			if (li->refCount) {
				LosAdd(li);
			}
		}
	}
}

CR_REG_METADATA(CLosHandler,(
	CR_IGNORED(losMipLevel),
	CR_IGNORED(airMipLevel),
	CR_IGNORED(losDiv),
	CR_IGNORED(airDiv),
	CR_IGNORED(invLosDiv),
	CR_IGNORED(invAirDiv),
	CR_IGNORED(airSize),
	CR_IGNORED(losSize),
	CR_IGNORED(requireSonarUnderWater),

	CR_MEMBER(losAlgo),
	CR_MEMBER(losMaps),
	CR_MEMBER(airLosMaps),
	CR_MEMBER(instanceHash),
	CR_MEMBER(toBeDeleted),
	CR_MEMBER(delayQue),
	CR_POSTLOAD(PostLoad)
))

CR_REG_METADATA_SUB(CLosHandler,DelayedInstance, (
	CR_MEMBER(instance),
	CR_MEMBER(timeoutTime)))


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


CLosHandler* losHandler;


CLosHandler::CLosHandler() :
	losMaps(teamHandler->ActiveAllyTeams()),
	airLosMaps(teamHandler->ActiveAllyTeams()),
	// airAlgo(int2(airSizeX, airSizeY), -1e6f, 15, readMap->GetMIPHeightMapSynced(airMipLevel)),
	losMipLevel(modInfo.losMipLevel),
	airMipLevel(modInfo.airMipLevel),
	losDiv(SQUARE_SIZE * (1 << losMipLevel)),
	airDiv(SQUARE_SIZE * (1 << airMipLevel)),
	invLosDiv(1.0f / losDiv),
	invAirDiv(1.0f / airDiv),
	airSize(std::max(1, mapDims.mapx >> airMipLevel), std::max(1, mapDims.mapy >> airMipLevel)),
	losSize(std::max(1, mapDims.mapx >> losMipLevel), std::max(1, mapDims.mapy >> losMipLevel)),
	requireSonarUnderWater(),
	losAlgo(losSize, -1e6f, 15, readMap->GetMIPHeightMapSynced(losMipLevel))
{
	for (int a = 0; a < teamHandler->ActiveAllyTeams(); ++a) {
		losMaps[a].SetSize(losSize.x, losSize.y, true);
		airLosMaps[a].SetSize(airSize.x, airSize.y, false);
	}
}


CLosHandler::~CLosHandler()
{
	for (auto& bucket: instanceHash) {
		for (LosInstance* li: bucket) {
			delete li;
		}
		bucket.clear();
	}

}


void CLosHandler::MoveUnit(CUnit* unit, bool redoCurrent)
{
	SCOPED_TIMER("LOSHandler::MoveUnit");

	// NOTE: under normal circumstances, this only gets called if a unit
	// has moved to a new map square since its last SlowUpdate cycle, so
	// any units that changed position between enabling and disabling of
	// globalLOS and *stopped moving* will still provide LOS at their old
	// square *after* it is disabled (until they start moving again)
	if (gs->globalLOS[unit->allyteam])
		return;
	if (unit->losRadius <= 0 && unit->airLosRadius <= 0)
		return;

	unit->lastLosUpdate = gs->frameNum;

	const float3& losPos = unit->midPos;
	const int allyteam = unit->allyteam;

	const int2 base = GetLosSquare(losPos);
	const int2 baseAir = GetAirSquare(losPos);
	const int baseSquare = base.y * losSize.x + base.x;

	LosInstance* instance = NULL;
	if (redoCurrent) {
		if (!unit->los) {
			return;
		}
		instance = unit->los;
		CleanupInstance(instance);
		instance->losSquares.clear();
		instance->basePos = base;
		instance->baseSquare = baseSquare; //this could be a problem if several units are sharing the same instance
		instance->baseAirPos = baseAir;
	} else {
		if (unit->los && (unit->los->baseSquare == baseSquare)) {
			return;
		}

		FreeInstance(unit->los);
		const int hash = GetHashNum(unit);

		for (LosInstance* li: instanceHash[hash]) {
			if (li->baseSquare == baseSquare         &&
			    li->losSize    == unit->losRadius    &&
			    li->airLosSize == unit->airLosRadius &&
			    li->baseHeight == unit->losHeight    &&
			    li->allyteam   == allyteam) {
				AllocInstance(li);
				unit->los = li;
				return;
			}
		}

		instance = new LosInstance(
			unit->losRadius,
			unit->airLosRadius,
			allyteam,
			base,
			baseSquare,
			baseAir,
			hash,
			unit->losHeight
		);

		instanceHash[hash].push_back(instance);
		unit->los = instance;
	}

	LosAdd(instance);
}


void CLosHandler::RemoveUnit(CUnit* unit, bool delayed)
{
	if (delayed) {
		DelayedFreeInstance(unit->los);
	} else {
		FreeInstance(unit->los);
	}
	unit->los = nullptr;
}


void CLosHandler::LosAdd(LosInstance* li)
{
	assert(li);
	assert(teamHandler->IsValidAllyTeam(li->allyteam));

	if (li->losSize > 0) {
		if (li->losSquares.empty())
			losAlgo.LosAdd(li->basePos, li->losSize, li->baseHeight, li->losSquares);
		losMaps[li->allyteam].AddMapSquares(li->losSquares, li->allyteam, 1);
	}
	if (li->airLosSize > 0) { airLosMaps[li->allyteam].AddMapArea(li->baseAirPos, li->allyteam, li->airLosSize, 1); }
}


void CLosHandler::LosRemove(LosInstance* li)
{
	if (li->losSize > 0) { losMaps[li->allyteam].AddMapSquares(li->losSquares, li->allyteam, -1); }
	if (li->airSize > 0) { airLosMaps[li->allyteam].AddMapArea(li->baseAirPos, li->allyteam, li->airLosSize, -1); }
}


void CLosHandler::FreeInstance(LosInstance* instance)
{
	if (instance == nullptr)
		return;

	instance->refCount--;
	if (instance->refCount > 0) {
		return;
	}

	LosRemove(instance);

	if (!instance->toBeDeleted) {
		instance->toBeDeleted = true;
		toBeDeleted.push_back(instance);
	}

	// reached max cache size, free one instance
	if (toBeDeleted.size() > 500) {
		LosInstance* li = toBeDeleted.front();
		toBeDeleted.pop_front();

		if (li->hashNum >= LOSHANDLER_MAGIC_PRIME || li->hashNum < 0) {
			LOG_L(L_WARNING,
					"[LosHandler::FreeInstance][2] bad LOS-instance hash (%d)",
					li->hashNum);
			return;
		}

		li->toBeDeleted = false;

		if (li->refCount == 0) {
			auto& cont = instanceHash[li->hashNum];
			auto it = std::find(cont.begin(), cont.end(), li);
			cont.erase(it);
			delete li;
		}
	}
}


int CLosHandler::GetHashNum(CUnit* unit)
{
	const unsigned int t =
		(unit->mapSquare * unit->losRadius + unit->allyteam) ^
		(*(unsigned int*) &unit->losHeight);
	//! hash-value range is [0, LOSHANDLER_MAGIC_PRIME - 1]
	return (t % LOSHANDLER_MAGIC_PRIME);
}


void CLosHandler::AllocInstance(LosInstance* instance)
{
	if (instance->refCount == 0) {
		LosAdd(instance);
	}
	instance->refCount++;
}


void CLosHandler::Update()
{
	while (!delayQue.empty() && delayQue.front().timeoutTime < gs->frameNum) {
		FreeInstance(delayQue.front().instance);
		delayQue.pop_front();
	}
}


void CLosHandler::DelayedFreeInstance(LosInstance* instance)
{
	DelayedInstance di;
	di.instance = instance;
	di.timeoutTime = (gs->frameNum + (GAME_SPEED + (GAME_SPEED >> 1)));

	delayQue.push_back(di);
}


bool CLosHandler::InLos(const CUnit* unit, int allyTeam) const
{
	// NOTE: units are treated differently than world objects in two ways:
	//   1. they can be cloaked (has to be checked BEFORE all other cases)
	//   2. when underwater, they are only considered to be in LOS if they
	//      are also in radar ("sonar") coverage if requireSonarUnderWater
	//      is enabled --> underwater units can NOT BE SEEN AT ALL without
	//      active radar!
	if (modInfo.alwaysVisibleOverridesCloaked) {
		if (unit->alwaysVisible)
			return true;
		if (unit->isCloaked)
			return false;
	} else {
		if (unit->isCloaked)
			return false;
		if (unit->alwaysVisible)
			return true;
	}

	// isCloaked always overrides globalLOS
	if (gs->globalLOS[allyTeam])
		return true;
	if (unit->useAirLos)
		return (InAirLos(unit->pos, allyTeam) || InAirLos(unit->pos + unit->speed, allyTeam));

	if (requireSonarUnderWater) {
		if (unit->IsUnderWater() && !radarHandler->InRadar(unit, allyTeam)) {
			return false;
		}
	}

	return (InLos(unit->pos, allyTeam) || InLos(unit->pos + unit->speed, allyTeam));
}
