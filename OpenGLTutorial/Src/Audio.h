/**
*	@file Audio.h
*/
#pragma once

namespace Audio
{
	static const size_t playerMax = 8;

	bool Initialize(const char* acfPath, const char* acbPath, const char* acwPath, const char* bspBusName);

	void Updaye();
	void Destroy();
	void Play(int playerId, int cueId);
	void Stop(int playerId);

}
