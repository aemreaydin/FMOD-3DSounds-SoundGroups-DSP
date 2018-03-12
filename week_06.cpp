#include "common\src\utils\utils.h"

#include <fmod.hpp>
#include <fmod_errors.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

FMOD_RESULT mResult;

FMOD::System* mSystem = NULL;

const int numSounds = 12;
FMOD::Channel *mChannels[numSounds], *m3DChannels[3];
FMOD::Sound *mSounds[numSounds], *m3DSounds[3];

FMOD_VECTOR mListenerPosition = { 44.0f, 0.0f, 0.0f };
FMOD_VECTOR mForward = { 0.0f, 0.0f, 1.0f };
FMOD_VECTOR mUp = { 0.0f, 1.0f, 0.0f };
FMOD_VECTOR mVelocity = { 0.0f, 0.0f, 0.0f };

FMOD_VECTOR* soundPosition[3];
FMOD_VECTOR* soundVelocity[3];

FMOD::ChannelGroup *groupA, *groupB, *groupC, *masterGroup;

FMOD::DSP* lowPass = 0;
FMOD::DSP* highPass = 0;
FMOD::DSP* echo = 0;
FMOD::DSP* flange = 0;
FMOD::DSP* distortion = 0;
FMOD::DSP* chorus = 0;
FMOD::DSP* fft = 0;
FMOD::DSP* fader = 0;
FMOD::DSP* pitchShift = 0;

bool bEscape = false;
bool bKeyDown = false;
int mCurrentSound = 0;
int mCurrent3DSound = 0;
bool b3DResume = true;

float currentPan[numSounds];
const int numStoryLine = 10;
std::string mStory[numStoryLine];
int currentChannelIndex = 0;
int storyChapter = 0;


void initMod();
void checkError(FMOD_RESULT result);
void handleKeyboard();
void importSounds(std::string filename);
void importStory();


int main()
{
	initMod();

	importStory();
	importSounds("../common/assets/soundFiles.txt" );


	for (int i = 0; i < 3; i++)
	{
		soundPosition[i] = new FMOD_VECTOR({ i * 44.0f, 0.0f, 0.0f });
		soundVelocity[i] = new FMOD_VECTOR({ 0.0f, 0.0f, 0.0f });
	}

	//For 3D sounds
	//*********************************************************************************
	// Create and Set the first sound
	mResult = mSystem->createSound("../common/assets/guitarAmbient.mp3", FMOD_3D, 0, &m3DSounds[0]);
	checkError(mResult);
	mResult = m3DSounds[0]->setMode(FMOD_LOOP_NORMAL);
	checkError(mResult);
	mResult = mSystem->playSound(m3DSounds[0], 0, true, &m3DChannels[0]);
	checkError(mResult);
	mResult = m3DChannels[0]->set3DAttributes(soundPosition[0], soundVelocity[0]);
	checkError(mResult);
	mResult = m3DChannels[0]->setPaused(false);
	checkError(mResult);

	// Create and Set the second sound
	mResult = mSystem->createSound("../common/assets/pianoCello.mp3", FMOD_3D, 0, &m3DSounds[1]);
	checkError(mResult);
	mResult = m3DSounds[1]->setMode(FMOD_LOOP_NORMAL);
	checkError(mResult);
	mResult = mSystem->playSound(m3DSounds[1], 0, true, &m3DChannels[1]);
	checkError(mResult);
	mResult = m3DChannels[1]->set3DAttributes(soundPosition[1], soundVelocity[1]);
	checkError(mResult);
	mResult = m3DChannels[1]->setPaused(false);
	checkError(mResult);

	// Create and Set the third sound
	mResult = mSystem->createSound("../common/assets/tribalDrums.wav", FMOD_3D, 0, &m3DSounds[2]);
	checkError(mResult);
	mResult = m3DSounds[2]->setMode(FMOD_LOOP_NORMAL);
	checkError(mResult);
	mResult = mSystem->playSound(m3DSounds[2], 0, true, &m3DChannels[2]);
	checkError(mResult);
	mResult = m3DChannels[2]->set3DAttributes(soundPosition[2], soundVelocity[2]);
	checkError(mResult);
	mResult = m3DChannels[2]->setPaused(false);
	checkError(mResult);


	//for DSP
	//*********************************************************************************
	// Create and Set sounds and channel groups
	// Create the master channel
	mResult = mSystem->getMasterChannelGroup(&masterGroup);
	checkError(mResult);
	//Create channel groups
	mResult = mSystem->createChannelGroup("Group A", &groupA);
	checkError(mResult);
	mResult = mSystem->createChannelGroup("Group B", &groupB);
	checkError(mResult);
	mResult = mSystem->createChannelGroup("Group C", &groupC);
	checkError(mResult);
	// Add channel groups to master
	mResult = masterGroup->addGroup(groupA);
	checkError(mResult);
	mResult = masterGroup->addGroup(groupB);
	checkError(mResult);
	mResult = masterGroup->addGroup(groupC);
	checkError(mResult);

	// Create DSP effects
	mResult = mSystem->createDSPByType(FMOD_DSP_TYPE_LOWPASS, &lowPass);
	checkError(mResult);
	mResult = mSystem->createDSPByType(FMOD_DSP_TYPE_HIGHPASS, &highPass);
	checkError(mResult);
	mResult = mSystem->createDSPByType(FMOD_DSP_TYPE_ECHO, &echo);
	checkError(mResult);
	mResult = mSystem->createDSPByType(FMOD_DSP_TYPE_FLANGE, &flange);
	checkError(mResult);
	mResult = mSystem->createDSPByType(FMOD_DSP_TYPE_DISTORTION, &distortion);
	checkError(mResult);
	mResult = mSystem->createDSPByType(FMOD_DSP_TYPE_CHORUS, &chorus);
	checkError(mResult);
	mResult = mSystem->createDSPByType(FMOD_DSP_TYPE_FFT, &fft);
	checkError(mResult);
	mResult = mSystem->createDSPByType(FMOD_DSP_TYPE_FADER, &fader);
	checkError(mResult);
	mResult = mSystem->createDSPByType(FMOD_DSP_TYPE_PITCHSHIFT, &pitchShift);
	checkError(mResult);

	//TODO: Change later
	/*mResult = groupA->addDSP(0, lowPass);
	checkError(mResult);
	mResult = groupA->addDSP(0, highPass);
	checkError(mResult);
	mResult = groupA->addDSP(0, echo);
	checkError(mResult);

	mResult = groupB->addDSP(0, flange);
	checkError(mResult);
	mResult = groupB->addDSP(0, distortion);
	checkError(mResult);
	mResult = groupB->addDSP(0, chorus);
	checkError(mResult);

	mResult = groupC->addDSP(0, fft);
	checkError(mResult);
	mResult = groupC->addDSP(0, fader);
	checkError(mResult);
	mResult = groupC->addDSP(0, pitchShift);
	checkError(mResult);*/



	// Play sounds as paused
	for (int i = 0; i < numSounds; i++)
	{
		mResult = mSystem->playSound(mSounds[i], 0, true, &mChannels[i]);
		checkError(mResult);
	}

	//set each channels to groups
	//Group A
	mResult = mChannels[0]->setChannelGroup(groupA);
	checkError(mResult);
	mResult = mChannels[3]->setChannelGroup(groupA);
	checkError(mResult);
	mResult = mChannels[4]->setChannelGroup(groupA);
	checkError(mResult);
	mResult = mChannels[5]->setChannelGroup(groupA);
	checkError(mResult);
	//Group B
	mResult = mChannels[1]->setChannelGroup(groupB);
	checkError(mResult);
	mResult = mChannels[6]->setChannelGroup(groupB);
	checkError(mResult);
	mResult = mChannels[7]->setChannelGroup(groupB);
	checkError(mResult);
	mResult = mChannels[8]->setChannelGroup(groupB);
	checkError(mResult);
	//Group C
	mResult = mChannels[2]->setChannelGroup(groupC);
	checkError(mResult);
	mResult = mChannels[9]->setChannelGroup(groupC);
	checkError(mResult);
	mResult = mChannels[10]->setChannelGroup(groupC);
	checkError(mResult);
	mResult = mChannels[11]->setChannelGroup(groupC);
	checkError(mResult);

	mResult = lowPass->setBypass(true);
	checkError(mResult);
	mResult = highPass->setBypass(true);
	checkError(mResult);
	mResult = echo->setBypass(true);
	checkError(mResult);
	mResult = flange->setBypass(true);
	checkError(mResult);
	mResult = distortion->setBypass(true);
	checkError(mResult);
	mResult = chorus->setBypass(true);
	checkError(mResult);
	mResult = fft->setBypass(true);
	checkError(mResult);
	mResult = fader->setBypass(true);
	checkError(mResult);
	mResult = pitchShift->setBypass(true);
	checkError(mResult);

	// Create the variables for the channel groups
	float currentVolume = 0.0f;
	float currentFrequency = 0.0f;
	float currentPitch = 0.0f;
	bool currentPaused = false;
	bool isFinished;

	while (!bEscape)
	{
		start_text();
		handleKeyboard();

		// Set 3D Listener Attributes
		mResult = mSystem->set3DListenerAttributes(0, &mListenerPosition, &mVelocity, &mForward, &mUp);
		checkError(mResult);
		// Update the system
		mResult = mSystem->update();
		checkError(mResult);

		// Create the room for the 3D sounds
		char room[90] = ".........................................................................................";
		room[(int)(soundPosition[0]->x)] = '1';
		room[(int)(soundPosition[1]->x)] = '2';
		room[(int)(soundPosition[2]->x)] = '3';
		room[(int)(mListenerPosition.x)] = 'L';

		print_text("Welcome to this place that I've yet to decide.");
		print_text("Media Fundamentals - Assignment #2");
		print_text("Created by Emre Aydin, Jessica Gunawan and Akash Mehta");
		print_text("HELP:");
		print_text("ALL --- Press ENTER to change between 3D and DSP sounds.");
		print_text("3D --- Change between sounds with SHIFT.");
		print_text("3D --- Change the position of selected sound by A-D.");
		print_text("3D --- Change the position of the listener by RIGHT and LEFT ARROWS.");
		print_text("DSP --- 0-8 chooses a specific sound, or use TAB to change the sounds one by one.");
		print_text("DSP --- UP ARROW increases the volume, DOWN ARROW decreases it.");
		print_text("DSP --- RIGHT ARROW increases the frequency, LEFT ARROW decreases it.");
		print_text("Press P to start the story (Make sure Current Section = Sound Group, not 3D)"); 
		print_text("");
		print_text("");
		print_text("Current Section: %s", b3DResume ? "3D Sounds	" : "Sound Groups");
		print_text("");

		print_text(room);
		print_text("");
		print_text("Current 3D Sound: %d", mCurrent3DSound + 1);
		print_text("3D Sound #1 Position X: %.2f", soundPosition[0]->x);
		print_text("3D Sound #2 Position X: %.2f", soundPosition[1]->x);
		print_text("3D Sound #3 Position X: %.2f", soundPosition[2]->x);
		print_text("Listener Position X: %.2f", mListenerPosition.x);
		print_text("");
		print_text("");


		/*mResult = mChannels[mCurrentSound]->isPlaying(&isFinished);
		checkError(mResult);*/
		mResult = mChannels[mCurrentSound]->getVolume(&currentVolume);
		checkError(mResult);
		mResult = mChannels[mCurrentSound]->getFrequency(&currentFrequency);
		checkError(mResult);
		mResult = mChannels[mCurrentSound]->getPitch(&currentPitch);
		checkError(mResult);
		mResult = mChannels[mCurrentSound]->getPaused(&currentPaused);
		checkError(mResult);

		print_text("Channel Groups and Sound Properties");
		print_text("Current Channel: %d\t", mCurrentSound);
		print_text("Current Sound %s", currentPaused ? "is paused\t\t" : "is not paused");
		print_text("Channel Volume: %%%.02f\t", currentVolume * 100);
		print_text("Channel Frequency: %.02f\t", currentFrequency);
		print_text("Channel Pitch: %.02f\t", currentPitch);

		//Story Time
		//Story Part 1
		if (storyChapter == 1) // 0 - 3 - 4 - 5
		{
			std::cout << "\n" << std::endl;
			std::cout << "Story Time" << std::endl;
			Sleep(1000);

			//print storyline
			std::cout << mStory[0] << std::endl;
			std::cout << mStory[1] << std::endl;
			std::cout << mStory[2] << std::endl;
			std::cout << mStory[3] << std::endl;
			
			mResult = mChannels[0]->setPaused(false);
			checkError(mResult);
			Sleep(1000);
			mResult = mChannels[3]->setPaused(false);
			checkError(mResult);
			Sleep(1000);
			mResult = mChannels[4]->setPaused(false);
			checkError(mResult);
			Sleep(2200);
			mResult = mChannels[5]->setPaused(false);
			checkError(mResult);

			for (int i = 0; i < 6; i++)
			{
				mResult = mChannels[3]->setPosition(0, FMOD_TIMEUNIT_MS);
				checkError(mResult);
				Sleep(2000);
			}

			mResult = mChannels[4]->setPosition(0, FMOD_TIMEUNIT_MS);
			checkError(mResult);
			Sleep(2300);
		
			mResult = mChannels[5]->setPosition(0, FMOD_TIMEUNIT_MS);
			checkError(mResult);
			Sleep(1800);
	
			Sleep(1500);
			//pause previous channels
			mResult = mChannels[0]->setPaused(true);
			checkError(mResult);
			mResult = mChannels[1]->setPaused(true);
			checkError(mResult);
			mResult = mChannels[2]->setPaused(true);
			checkError(mResult);
			mResult = mChannels[3]->setPaused(true);
			checkError(mResult);

			mResult = mSystem->update();
			checkError(mResult);
			storyChapter++;
		}
		if (storyChapter == 2) // 1 - 6 - 7 - 8
		{

			//print storyline
			std::cout << mStory[4] << std::endl;
			std::cout << mStory[5] << std::endl;
			std::cout << mStory[6] << std::endl;

			Sleep(1000);
			mResult = mChannels[1]->setPaused(false);
			checkError(mResult);
			Sleep(1000);

			mResult = mChannels[7]->setPaused(false);
			checkError(mResult);
			Sleep(1000);
			mResult = mChannels[8]->setPaused(false);
			checkError(mResult);
			Sleep(1500);
			mResult = mChannels[6]->setPaused(false);
			checkError(mResult);
			Sleep(2000);

			for (int i = 0; i < 3; i++)
			{
				
			}

			for (int i = 0; i < 3; i++)
			{
				mResult = mChannels[6]->setPosition(0, FMOD_TIMEUNIT_MS);
				checkError(mResult);
				Sleep(1800);
				mResult = mChannels[7]->setPosition(0, FMOD_TIMEUNIT_MS);
				checkError(mResult);
				Sleep(1300);
				mResult = mChannels[8]->setPosition(0, FMOD_TIMEUNIT_MS);
				checkError(mResult);
				Sleep(1000);
			}


			Sleep(1500);
			//pause previous channels
			mResult = mChannels[1]->setPaused(true);
			checkError(mResult);
			mResult = mChannels[6]->setPaused(true);
			checkError(mResult);
			mResult = mChannels[7]->setPaused(true);
			checkError(mResult);
			mResult = mChannels[8]->setPaused(true);
			checkError(mResult);

			mResult = mSystem->update();
			checkError(mResult);
			storyChapter++;
		}
		if (storyChapter == 3) // 2 - 9 - 10 - 11
		{

			//print storyline
			std::cout << mStory[7] << std::endl;
			std::cout << mStory[8] << std::endl;
			std::cout << mStory[9] << std::endl;

			//Sleep(1000);
			mResult = mChannels[2]->setPaused(false);
			checkError(mResult);
			Sleep(1000);

			mResult = mChannels[9]->setPaused(false);
			checkError(mResult);
			Sleep(1000);
			mResult = mChannels[10]->setPaused(false);
			checkError(mResult);
			Sleep(1500);
			mResult = mChannels[11]->setPaused(false);
			checkError(mResult);
			Sleep(2000);

			for (int i = 0; i < 3; i++)
			{
				mResult = mChannels[9]->setPosition(0, FMOD_TIMEUNIT_MS);
				checkError(mResult);
				Sleep(1800);
				mResult = mChannels[10]->setPosition(0, FMOD_TIMEUNIT_MS);
				checkError(mResult);
				Sleep(1300);
				mResult = mChannels[11]->setPosition(0, FMOD_TIMEUNIT_MS);
				checkError(mResult);
				Sleep(1000);
			}


			Sleep(1500);
			//pause previous channels
			mResult = mChannels[2]->setPaused(true);
			checkError(mResult);
			mResult = mChannels[9]->setPaused(true);
			checkError(mResult);
			mResult = mChannels[10]->setPaused(true);
			checkError(mResult);
			mResult = mChannels[11]->setPaused(true);
			checkError(mResult);

			mResult = mSystem->update();
			checkError(mResult);
			storyChapter++;
		}
		

		for (int i = 0; i < numSounds; i++)
		{
			mResult = mChannels[i]->isPlaying(&isFinished);
			//checkError( gResult );
			if (!isFinished && (storyChapter == 0 || storyChapter == 3))
			{
				mResult = mSystem->playSound(mSounds[i], 0, true, &mChannels[i]);
				//gResult = gChannel[ i ]->setPosition( 0, FMOD_TIMEUNIT_MS );
				checkError(mResult);
				isFinished = true;
			}
		}


		



		//Story end



		end_text();
		Sleep(50);
	}

	for (int i = 0; i < 3; i++)
	{
		if (m3DSounds[i])
		{
			mResult = m3DSounds[i]->release();
			checkError(mResult);
		}
	}

	if (mSystem)
	{
		mResult = mSystem->close();
		checkError(mResult);
		mResult = mSystem->release();
		checkError(mResult);
	}
	return 0;
}

void initMod()
{
	// Create the System
	mResult = FMOD::System_Create(&mSystem);
	checkError(mResult);
	// Initialize the System
	mResult = mSystem->init(512, FMOD_INIT_NORMAL, 0);
	checkError(mResult);
}

void checkError(FMOD_RESULT result)
{
	if (result != FMOD_OK)
	{
		fprintf(stderr, "FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		exit(-1);
	}
}

//************************************************************************
// NEW

void importSounds(std::string filename)
{
	std::ifstream soundFile(filename);
	std::stringstream soundStream;
	std::string soundString;

	int streamIndex = 0;
	int sampleIndex = 0;

	soundFile >> soundString;
	while (soundString.compare("END"))
	{
		soundFile >> soundString;
		if (soundString == "Streaming")
		{
			soundFile >> soundString;
			soundString = "../common/assets/" + soundString;
			mResult = mSystem->createSound(soundString.c_str(), FMOD_CREATESTREAM, 0, &mSounds[streamIndex]);
			checkError(mResult);
			mResult = mSounds[streamIndex]->setMode(FMOD_LOOP_NORMAL);
			checkError(mResult);

			/*mResult = mSystem->createSound(soundString.c_str(), FMOD_3D, 0, &m3DSounds[streamIndex]);
			checkError(mResult);
			mResult = m3DSounds[streamIndex]->setMode(FMOD_LOOP_NORMAL);
			checkError(mResult);*/

			/*mResult = mSystem->playSound(mSounds[streamIndex], 0, true, &mChannels[streamIndex]);
			checkError(mResult);*/


			streamIndex++;
		}
		if (soundString == "Sampled")
		{
			soundFile >> soundString;
			soundString = "../common/assets/" + soundString;
			mResult = mSystem->createSound(soundString.c_str(), FMOD_CREATESAMPLE, 0, &mSounds[streamIndex]);
			checkError(mResult);
			mResult = mSounds[streamIndex]->setMode(FMOD_LOOP_OFF);
			checkError(mResult);

			/*mResult = mSystem->playSound(mSounds[streamIndex], 0, true, &mChannels[streamIndex]);
			checkError(mResult);*/

			streamIndex++;
		}
	}

}

void importStory()
{
	std::ifstream storyFile("../common/assets/storyFile.txt");
	for (int i = 0; i < numStoryLine; i++)
	{
		std::getline(storyFile, mStory[i]);
	}
}

//************************************************************************



void handleKeyboard()
{
	//Esc key pressed
	if (GetAsyncKeyState(VK_ESCAPE)) {
		bEscape = true;
	}

	//===============================================================================================
	// Space bar pressed		
	if ((GetKeyState(VK_SPACE) < 0) && !bKeyDown) {
		bKeyDown = true;

		if (!b3DResume)
		{
			bool isPaused = true;
			mResult = mChannels[mCurrentSound]->getPaused(&isPaused);
			checkError(mResult);
			mResult = mChannels[mCurrentSound]->setPaused(!isPaused);
			checkError(mResult);
		}

		Sleep(200);
		bKeyDown = false;
	}
	else if ((GetKeyState(VK_RETURN) < 0) && !bKeyDown)
	{
		bKeyDown = true;

		if (b3DResume)
		{
			for (int i = 0; i < 3; i++)
			{
				mResult = m3DChannels[i]->setPaused(true);
				checkError(mResult);
			}
			mResult = lowPass->setBypass(false);
			checkError(mResult);
			mResult = highPass->setBypass(false);
			checkError(mResult);
			mResult = echo->setBypass(false);
			checkError(mResult);
			mResult = flange->setBypass(false);
			checkError(mResult);
			mResult = distortion->setBypass(false);
			checkError(mResult);
			mResult = chorus->setBypass(false);
			checkError(mResult);
			mResult = fft->setBypass(false);
			checkError(mResult);
			mResult = fader->setBypass(false);
			checkError(mResult);
			mResult = pitchShift->setBypass(false);
			checkError(mResult);
		}
		else
		{
			for (int i = 0; i < 3; i++)
			{
				mResult = m3DChannels[i]->setPaused(false);;
				checkError(mResult);
			}
			mResult = lowPass->setBypass(true);
			checkError(mResult);
			mResult = highPass->setBypass(true);
			checkError(mResult);
			mResult = echo->setBypass(true);
			checkError(mResult);
			mResult = flange->setBypass(true);
			checkError(mResult);
			mResult = distortion->setBypass(true);
			checkError(mResult);
			mResult = chorus->setBypass(true);
			checkError(mResult);
			mResult = fft->setBypass(true);
			checkError(mResult);
			mResult = fader->setBypass(true);
			checkError(mResult);
			mResult = pitchShift->setBypass(true);
			checkError(mResult);
		}
		b3DResume = !b3DResume;

		Sleep(200);
		bKeyDown = false;
	}
	else if ((GetKeyState(VK_SHIFT) < 0) && !bKeyDown)
	{
		bKeyDown = true;

		if (b3DResume)
		{
			mCurrent3DSound++;
			if (mCurrent3DSound == 3)
				mCurrent3DSound = 0;
		}

		Sleep(200);
		bKeyDown = false;
	}
	else if ((GetKeyState(VK_TAB) < 0) && !bKeyDown)
	{
		bKeyDown = true;

		if (!b3DResume)
		{
			mCurrentSound++;
			if (mCurrentSound == 9)
				mCurrentSound = 0;
		}

		Sleep(200);
		bKeyDown = false;
	}

	//===============================================================================================
	//Arrow UP
	else if ((GetKeyState(VK_UP) < 0) && !bKeyDown) {
		bKeyDown = true;

		if (!b3DResume)
		{
			float volume;
			mResult = mChannels[mCurrentSound]->getVolume(&volume);
			checkError(mResult);

			if (volume < 1.0f)
				volume += 0.1f;
			mResult = mChannels[mCurrentSound]->setVolume(volume);
			checkError(mResult);
		}		

		Sleep(200);
		bKeyDown = false;
	}

	//===============================================================================================
	//Arrow Down
	else if ((GetKeyState(VK_DOWN) < 0) && !bKeyDown) {
		bKeyDown = true;

		if (!b3DResume)
		{
			float volume;
			mResult = mChannels[mCurrentSound]->getVolume(&volume);
			checkError(mResult);

			if (volume > 0.0f)
				volume -= 0.1f;
			mResult = mChannels[mCurrentSound]->setVolume(volume);
			checkError(mResult);
		}		

		Sleep(200);
		bKeyDown = false;
	}

	//===============================================================================================
	//Arrow right
	else if ((GetKeyState(VK_RIGHT) < 0) && !bKeyDown) {
		bKeyDown = true;

		if (b3DResume)
		{
			mListenerPosition.x += 1.0f;
			mListenerPosition.x = (mListenerPosition.x >= 88.0f) ? 88.0f : mListenerPosition.x;
		}
		else
		{
			float frequency;
			mResult = mChannels[mCurrentSound]->getFrequency(&frequency);
			checkError(mResult);

			frequency += 1000.0f;
			mResult = mChannels[mCurrentSound]->setFrequency(frequency);
			checkError(mResult);
		}

		Sleep(50);
		bKeyDown = false;
	}

	//===============================================================================================
	//Arrow left
	else if ((GetKeyState(VK_LEFT) < 0) && !bKeyDown) {
		bKeyDown = true;

		if (b3DResume)
		{
			mListenerPosition.x -= 1.0f;
			mListenerPosition.x = (mListenerPosition.x <= 0.0f) ? 0.0f : mListenerPosition.x;
		}
		else
		{
			float frequency;
			mResult = mChannels[mCurrentSound]->getFrequency(&frequency);
			checkError(mResult);

			frequency -= 1000.0f;
			mResult = mChannels[mCurrentSound]->setFrequency(frequency);
			checkError(mResult);
		}

		Sleep(50);
		bKeyDown = false;
	}

	//===============================================================================================
	//Number 0
	else if ((GetKeyState(0x30) < 0) && !bKeyDown) {
		bKeyDown = true;
		
		if(!b3DResume)
			mCurrentSound = 0;

		Sleep(200);
		bKeyDown = false;
	}

	//===============================================================================================
	//Number 1
	else if ((GetKeyState(0x31) < 0) && !bKeyDown) { //Key down
		bKeyDown = true;

		if (!b3DResume)
			mCurrentSound = 1;

		Sleep(200);
		bKeyDown = false;
	}

	//===============================================================================================
	//Number 2
	else if ((GetKeyState(0x32) < 0) && !bKeyDown) {
		bKeyDown = true;

		if (!b3DResume)
			mCurrentSound = 2;

		Sleep(200);
		bKeyDown = false;
	}

	//===============================================================================================
	//Number 3
	else if ((GetKeyState(0x33) < 0) && !bKeyDown) {
		bKeyDown = true;

		if (!b3DResume)
			mCurrentSound = 3;

		Sleep(200);
		bKeyDown = false;
	}
	else if ((GetKeyState(0x34) < 0) && !bKeyDown) {
		bKeyDown = true;

		if (!b3DResume)
			mCurrentSound = 4;

		Sleep(200);
		bKeyDown = false;
	}
	else if ((GetKeyState(0x35) < 0) && !bKeyDown) {
		bKeyDown = true;

		if (!b3DResume)
			mCurrentSound = 5;

		Sleep(200);
		bKeyDown = false;
	}
	else if ((GetKeyState(0x36) < 0) && !bKeyDown) {
		bKeyDown = true;

		if (!b3DResume)
			mCurrentSound = 6;

		Sleep(200);
		bKeyDown = false;
	}
	else if ((GetKeyState(0x37) < 0) && !bKeyDown) {
		bKeyDown = true;

		if (!b3DResume)
			mCurrentSound = 7;

		Sleep(200);
		bKeyDown = false;
	}
	else if ((GetKeyState(0x38) < 0) && !bKeyDown) {
		bKeyDown = true;

		if (!b3DResume)
			mCurrentSound = 8;

		Sleep(200);
		bKeyDown = false;
	}
	else if ((GetKeyState(0x41) < 0) && !bKeyDown) { //A Button
		bKeyDown = true;

		if (b3DResume)
		{
			soundPosition[mCurrent3DSound]->x -= 1.0f;
			mResult = m3DChannels[mCurrent3DSound]->set3DAttributes(soundPosition[mCurrent3DSound], soundVelocity[0]);
			checkError(mResult);
			soundPosition[mCurrent3DSound]->x = (soundPosition[mCurrent3DSound]->x <= 0.0f) ? 0.0f : soundPosition[mCurrent3DSound]->x;
		}

		Sleep(200);
		bKeyDown = false;
	}
	else if ((GetKeyState(0x44) < 0) && !bKeyDown) { //D Button
		bKeyDown = true;

		if (b3DResume)
		{
			soundPosition[mCurrent3DSound]->x += 1.0f;
			mResult = m3DChannels[mCurrent3DSound]->set3DAttributes(soundPosition[mCurrent3DSound], soundVelocity[0]);
			checkError(mResult);
			soundPosition[mCurrent3DSound]->x = (soundPosition[mCurrent3DSound]->x >= 88.0f) ? 88.0f : soundPosition[mCurrent3DSound]->x;
		}

		Sleep(200);
		bKeyDown = false;
	}
	else if ((GetKeyState(0x50) < 0) && !bKeyDown) { //P Button
		bKeyDown = true;

		//initialize sound
		for (int i = 0; i < numSounds; i++)
		{
			mResult = mChannels[i]->setPaused(true);
			checkError(mResult);
			mResult = mChannels[i]->setPosition(0, FMOD_TIMEUNIT_MS);
			checkError(mResult);
		}

		storyChapter++;

		Sleep(200);
		bKeyDown = false;
	}



}