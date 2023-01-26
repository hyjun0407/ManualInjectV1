#pragma once
#include <Windows.h> 
#include <iostream>
using namespace std;

struct offset {
	static uint64_t CodeCave;
	static uint64_t PlayerCameraManager_HookBase;
	static uint64_t PlayerCameraManager;
};

uint64_t offset::CodeCave = 0xEEA0000;
uint64_t offset::PlayerCameraManager_HookBase = 0x4aef530; //75 ? F2 0F 10 ? ? ? ? ? F2 0F 11 ? 8B ? ? ? ? ? 89 47 ?
uint64_t offset::PlayerCameraManager = 0x4b8;