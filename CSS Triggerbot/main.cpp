/*
This code allows us to create a Triggerbot hack for CSS,
in theory this will work for most games. 
Especially source based like TF2 all Counter-strikes etc.

Credits to ExcidiumDubstep for external base tutorial and Nubtik for certain code sections

Written by Fleep at http://guidedhacking.com/, feel free to check us out for many tutorials 
on cheating and join our friendly hacking community
If you end up releasing any hacks from this code they would be greatly appreciated on our GuidedHacking.
*/

#include <Windows.h>  
#include <iostream> 
#include "HackProcess.h" 

//Create our 'hooking' and process managing object
CHackProcess fProcess;  
using namespace std;  

//The player base is VERY important so we know where our player info is at
//including current jump status so we can use force jumping making our bHop
const DWORD Player_Base = 0x53FB04;//0x00574560;
//We use F6 to exit the hack
#define F6_Key 0x75
//used to alternate between shooting var
bool b_ShotNow = false;
//The ATTACK address BELOW, WE WRITE 5 TO SHOOT OR 4 TO 
const DWORD dw_Attack = 0x56C5C0;//client
const DWORD dw_mTeamOffset = 0x98;//client

//Here we overwrite the value for +ATTACK1 and -ATTACK1
//so we can keep shooting at the enemy when crosshair ID changes
int i_Shoot = 5;
int i_DontShoot = 4;

//Here we store the num of players and update it regularly to know how many enemies we are dealing with
//this saves us doing loops unless their necessary e.g. we have 12 players and still loop 32 times wasting our great resources
//This makes our triggerbot MUCH faster in general
int NumOfPlayers = 32;
const DWORD dw_PlayerCountOffs = 0x5CE10C;//Engine.dll
const DWORD dw_crosshairOffs = 0x14D4;//Client.dll

//Enemy Vars including the enemy loop
const DWORD EntityPlayer_Base = 0x54D324;

//How far in memory is each enemy data
const DWORD EntityLoopDistance = 0x10;


//We will use this struct throughout all other tutorials adding more variables every time
struct MyPlayer_t  
{ 
	//Player base
	DWORD CLocalPlayer; 
	//Get our player's team
	int Team; 
	//stores our current entity in sight
	int CrosshairEntityID;
	void ReadInformation() 
	{
		// Reading CLocalPlayer Pointer to our "CLocalPlayer" DWORD. 
		ReadProcessMemory (fProcess.__HandleProcess, (PBYTE*)(fProcess.__dwordClient + Player_Base), &CLocalPlayer, sizeof(DWORD), 0);
		// Reading out our Team to our "Team" Varible. 
		ReadProcessMemory (fProcess.__HandleProcess, (PBYTE*)(CLocalPlayer + dw_mTeamOffset), &Team, sizeof(int), 0);

		//1478 IS THE CURRENT ENEMY/ENTITY IN CROSSHAIRS
		//this was found by scanning barrels and enemies constantly through Changed and Unchanged values in cheat engine.
		//then as usual address - base gave the offset answer
		ReadProcessMemory (fProcess.__HandleProcess, (PBYTE*)(CLocalPlayer + dw_crosshairOffs), &CrosshairEntityID, sizeof(int), 0); 

		//Here we find how many player entities exist in our game, through this we make sure to only loop the amount of times we need
		//when grabbing player data
		//Note that this call could be even better at a regular 15 or so seconds timer but performance shouldn't vary a great deal
		ReadProcessMemory (fProcess.__HandleProcess, (PBYTE*)(fProcess.__dwordEngine + dw_PlayerCountOffs), &NumOfPlayers, sizeof(int), 0); 
	}
}MyPlayer; 


//Stores our enemy data
struct PlayerList_t 
{
	DWORD CBaseEntity; 
	int Team; 

	void ReadInformation(int Player) 
	{
		// Reading CBaseEntity Pointer to our "CBaseEntity" DWORD + Current Player in the loop. 0x10 is the CBaseEntity List Size 
		//"client.dll"+00545204 //0x571A5204
		ReadProcessMemory (fProcess.__HandleProcess, (PBYTE*)(fProcess.__dwordClient + EntityPlayer_Base  + (Player * EntityLoopDistance)),
			&CBaseEntity, sizeof(DWORD), 0);
		// Reading out our Team to our "Team" Varible. 
		ReadProcessMemory (fProcess.__HandleProcess, (PBYTE*)(CBaseEntity + dw_mTeamOffset), &Team, sizeof(int), 0);
 	}
}PlayerList[32];  



void TriggerBot()
{
	//disable the ATTACK1 so we can shoot again next time
	//just took a shot(+attack), we need to reset the var back to be able to shoot again
	if(!b_ShotNow)
	{
		//bHOP bay bay, SWITCH off jump
		WriteProcessMemory(fProcess.__HandleProcess, (int*)(fProcess.__dwordClient + dw_Attack), &i_DontShoot, sizeof(int), NULL);
		b_ShotNow = !b_ShotNow;
	}

	//If we are not aiming at anyone or an unwanted entity such as a barrel then return
	if(MyPlayer.CrosshairEntityID == 0)
		return;
	
	//If player is on the same team we skip the triggerbot and DONT shoot
	//-1 because in game the crosshair entity Id's start from 1 instead of 0 in as in the array
	//we take 1 away so it accesses our correct Player Array
	if (PlayerList[MyPlayer.CrosshairEntityID-1].Team == MyPlayer.Team) 
		return;

	//if entity in crosshair is greater than OUR NUMBER of players it means its not an enemy
	if(MyPlayer.CrosshairEntityID > NumOfPlayers)
		return;


	//Everything is as we want so shoot
	//TAKE A SHOT once we know the +attack command has been reset
	if(b_ShotNow)
	{
		//SHOOT
		WriteProcessMemory(fProcess.__HandleProcess, (int*)(fProcess.__dwordClient + dw_Attack), &i_Shoot, sizeof(int), NULL);
		b_ShotNow = !b_ShotNow;
	}
}


//Our main loop
int main() 
{   
	fProcess.RunProcess(); // Waiting for CSS......
	cout << "Game found! Running Triggerbot..." << endl; 

	//Exit if the F6 key is pressed
	while (!GetAsyncKeyState(F6_Key)) // or for(;;)
	{
		//Read player information into our struct so we know the player 
		//base and the bunny hop addresses
		MyPlayer.ReadInformation(); 

		//Loop through all our players and retrieve their information
		//we use this mainly here to find out what players are our enemies and what players are on our team
		for(int i = 0; i < NumOfPlayers; i ++)
		{
			PlayerList[i].ReadInformation(i);

			//std::cout << PlayerList[i].Team << std::endl;

		}

		//Perform our Triggerbot
		TriggerBot();
	}
}