/***************************************************************************
                          playerportaudio.cpp  -  description
                             -------------------
    begin                : Wed Feb 20 2002
    copyright            : (C) 2002 by Tue and Ken Haste Andersen
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "playerportaudio.h"

PlayerPortAudio::PlayerPortAudio(int size, std::vector<EngineObject *> *engines) : Player(size, engines)
{
    PaError err;
    err = Pa_Initialize();
    if( err != paNoError ) qFatal("PortAudio initialization error");
    err = Pa_OpenStream(&stream,
                        paNoDevice,     // default input device
                        0,              // no input
                        paInt16,      
                        NULL,
                        Pa_GetDefaultOutputDeviceID(), // default output device
                        NO_CHANNELS,              // stereo output
                        paInt16,    
                        NULL,
                        (double)SRATE,
                        size/NO_CHANNELS, 	// frames per buffer per channel
                        0,              // number of buffers, if zero then use default minimum
                        paClipOff,      // we won't output out of range samples so don't bother clipping them
                        paCallback,
                        this );

    if( err != paNoError ) qFatal("PortAudio open stream error: %s",Pa_GetErrorText(err) );

    buffer_size = size;
    qDebug("Using PortAudio. Buffer size : %i samples.",buffer_size);
    allocate();
}

PlayerPortAudio::~PlayerPortAudio()
{
	Pa_Terminate();
}

void PlayerPortAudio::start(EngineObject *_reader)
{
    Player::start(_reader);
    PaError err = Pa_StartStream( stream );
    if( err != paNoError ) qFatal("PortAudio start stream error: %s", Pa_GetErrorText(err));
}

void PlayerPortAudio::wait()
{
}

void PlayerPortAudio::stop()
{
	PaError err = Pa_StopStream( stream );
	if( err != paNoError ) exit(-1);

	err = Pa_CloseStream( stream );
	if( err != paNoError ) exit(-1);
}

/* -------- ------------------------------------------------------
   Purpose: Wrapper function to call processing loop function,
            implemented as a method in a class. Used in PortAudio,
            which knows nothing about C++.
   Input:   .
   Output:  -
   -------- ------------------------------------------------------ */
static int paCallback(void *inputBuffer, void *outputBuffer,
                      unsigned long framesPerBuffer,
                      PaTimestamp outTime, void *_player)
{
    Player *player = (Player *)_player;
    SAMPLE *out = (SAMPLE*)outputBuffer;
    player->prepareBuffer();
    for (int i=0; i<framesPerBuffer*NO_CHANNELS; i++) 
        *out++=player->out_buffer[i];
    return 0;
}
