#ifndef SPROG_AUDIO_H
#define SPROG_AUDIO_H

typedef struct
{
	DWORD RiffId;
	DWORD RiffChunkSize;
	DWORD WaveId;

	DWORD FormatId;
	DWORD FormatChunkSize;
	DWORD FormatCode;
	DWORD NChannel;
	DWORD SampleRate;
	DWORD ByteRate;
	DWORD BlockAlign;
	DWORD BitPerSample;

	DWORD DataId;
	DWORD DataChunkSize;
	WORD Samples;
} WAV, *HWAV;

typedef struct
{
	double PlaybackTime;
	UINT32 PlaybackSample;
	HWAV Wave;
	WORD* Samples;
} AudioContext, *HAudioContext;

typedef struct __AudioSystem
{
	UINT32 BufferSize;
	IAudioClient2* AudioClient;
	IAudioRenderClient* RenderClient;

	HRESULT (*Initialize)(struct __AudioSystem* this);
	HRESULT (*Frame)(struct __AudioSystem* this, HAudioContext context);
	HRESULT (*Release)(struct __AudioSystem* this);
} AudioSystem, *HAudioSystem;

void AudioContext_new(HAudioContext handle, void* data);
void AudioSystem_new(HAudioSystem handle);

#endif //SPROG_AUDIO_H
