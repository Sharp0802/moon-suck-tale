#include "pch.h"
#include "audio.h"

static HRESULT InitializeAudioSystem(HAudioSystem lphAudioSystem)
{
	HRESULT hr;

	IMMDeviceEnumerator* devEnum;
	hr = CoCreateInstance(
			&CLSID_MMDeviceEnumerator,
			NULL,
			CLSCTX_ALL,
			&IID_IMMDeviceEnumerator,
			(LPVOID*)&devEnum);
	if (FAILED(hr))
		return hr;

	IMMDevice* dev;
	hr = devEnum->lpVtbl->GetDefaultAudioEndpoint(devEnum, eRender, eConsole, &dev);
	if (FAILED(hr))
		return hr;

	devEnum->lpVtbl->Release(devEnum);

	IAudioClient2* client;
	hr = dev->lpVtbl->Activate(dev, &IID_IAudioClient2, CLSCTX_ALL, NULL, (LPVOID*)&client);
	if (FAILED(hr))
		return hr;

	dev->lpVtbl->Release(dev);

	/*
	 * only PCM
	 * only 2-channel
	 * only 44100Hz
	 * only 16bit sample
	 */

	WAVEFORMATEX format;
	memset(&format, 0, sizeof format);

	format.wFormatTag = WAVE_FORMAT_PCM;
	format.nChannels = 2;
	format.nSamplesPerSec = 44100;
	format.wBitsPerSample = 16;
	format.nBlockAlign = (format.nChannels * format.wBitsPerSample) / 8;
	format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;

	const REFERENCE_TIME REFTIMES_PER_SEC = 10000000;
	REFERENCE_TIME duration = REFTIMES_PER_SEC * 2;
	DWORD flags = AUDCLNT_STREAMFLAGS_RATEADJUST |
				  AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM |
				  AUDCLNT_STREAMFLAGS_SRC_DEFAULT_QUALITY;
	hr = client->lpVtbl->Initialize(
			client,
			AUDCLNT_SHAREMODE_SHARED,
			flags,
			duration,
			0,
			&format,
			NULL);
	if (FAILED(hr))
		goto E_REQ_REL;

	IAudioRenderClient* renderer;
	hr = client->lpVtbl->GetService(client, &IID_IAudioRenderClient, (LPVOID*)&renderer);
	if (FAILED(hr))
		goto E_REQ_REL;

	UINT32 bufferSize;
	hr = client->lpVtbl->GetBufferSize(client, &bufferSize);
	if (FAILED(hr))
		goto E_REQ_REL_RDR;

	hr = client->lpVtbl->Start(client);
	if (FAILED(hr))
		goto E_REQ_REL_RDR;

	lphAudioSystem->BufferSize = bufferSize;
	lphAudioSystem->AudioClient = client;
	lphAudioSystem->RenderClient = renderer;

	return S_OK;

E_REQ_REL_RDR:
	renderer->lpVtbl->Release(renderer);
E_REQ_REL:
	client->lpVtbl->Release(client);
	return hr;
}

static HRESULT FrameAudioSystem(HAudioSystem lphAudioSystem, HAudioContext lphAudioContext)
{
	UINT32 nSamples = lphAudioContext->Wave->DataChunkSize / (lphAudioContext->Wave->NChannel * sizeof(UINT16));

	HRESULT hr;
	UINT32 padding;

	hr = lphAudioSystem->AudioClient->lpVtbl->GetCurrentPadding(lphAudioSystem->AudioClient, &padding);
	if (FAILED(hr))
		return hr;

	UINT32 latency = lphAudioSystem->BufferSize / 50;
	UINT32 nFrames = latency - padding;

	UINT16* buffer;
	hr = lphAudioSystem->RenderClient->lpVtbl->GetBuffer(lphAudioSystem->RenderClient, nFrames, (BYTE**)&buffer);
	if (FAILED(hr))
		return hr;

	for (UINT32 i = 0; i < nFrames; ++i)
	{
		UINT32 left = lphAudioContext->Wave->NChannel * lphAudioContext->PlaybackSample;
		UINT32 right = left + lphAudioContext->Wave->NChannel - 1;

		UINT16 lSample = lphAudioContext->Samples[left];
		UINT16 rSample = lphAudioContext->Samples[right];

		++lphAudioContext->PlaybackSample;

		*buffer++ = lSample;
		*buffer++ = rSample;

		if (lphAudioContext->PlaybackSample >= nSamples)
			lphAudioContext->PlaybackSample += nSamples;
	}

	hr = lphAudioSystem->RenderClient->lpVtbl->ReleaseBuffer(lphAudioSystem->RenderClient, nFrames, 0);
	if (FAILED(hr))
		return hr;

	IAudioClock* clock;
	hr = lphAudioSystem->AudioClient->lpVtbl->GetService(lphAudioSystem->AudioClient, &IID_IAudioClock, (LPVOID*)&clock);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

static HRESULT ReleaseAudioSystem(HAudioSystem lphAudioSystem)
{
	lphAudioSystem->RenderClient->lpVtbl->Release(lphAudioSystem->RenderClient);
	lphAudioSystem->AudioClient->lpVtbl->Release(lphAudioSystem->AudioClient);

	return S_OK;
}

void AudioContext_new(HAudioContext handle, void* data)
{
	handle->PlaybackTime = 0;
	handle->PlaybackSample = 0;
	handle->Wave = (HWAV)data;
	handle->Samples = &handle->Wave->Samples;
}

void AudioSystem_new(HAudioSystem handle)
{
	handle->Initialize = InitializeAudioSystem;
	handle->Frame = FrameAudioSystem;
	handle->Release = ReleaseAudioSystem;
}
