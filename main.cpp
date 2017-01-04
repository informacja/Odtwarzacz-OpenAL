#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <Windows.h>

#include <AL/al.h>
#include <AL/alc.h>

#define PATH "path"
// plik musi byc odpowiednio skonwertowany nie moze miec meta daty 
// znalaz³em juz kiedys taki konwenter online http://media.io/
#define LOOP 0 
// zapêtlanie sie do puki nie nacisnie sie esc
#define GAIN 0.5f
// stopen glosnosci

using namespace std;

// jezeli zwruci true to zacznie grac muzyka
bool foo(){
	return true;
}

struct sound{
	ALenum format;
	DWORD size;
	unsigned char* data;
	ALuint freq;

	void fr(){
		free(this->data);
	}

};

int load(string path, sound& s){
	FILE* f = fopen(path.c_str(), "rb");

	char type[4];
	DWORD size, chunkSize;
	short formatType, channels;
	DWORD sampleRate, avgBytesPerSec;
	short bytesPerSample, bitsPerSample;
	DWORD dataSize;

	fread(type, sizeof(char), 4, f);
	if (type[0] != 'R' || type[1] != 'I' || type[2] != 'F' || type[3] != 'F'){
		fclose(f);
		return 2;
	}

	fread(&size, sizeof(DWORD), 1, f);
	fread(type, sizeof(char), 4, f);
	if (type[0] != 'W' || type[1] != 'A' || type[2] != 'V' || type[3] != 'E'){
		fclose(f);
		return 3;
	}

	fread(type, sizeof(char), 4, f);
	if (type[0] != 'f' || type[1] != 'm' || type[2] != 't' || type[3] != ' '){
		fclose(f);
		return 4;
	}

	fread(&chunkSize, sizeof(DWORD), 1, f);
	fread(&formatType, sizeof(short), 1, f);
	fread(&channels, sizeof(short), 1, f);
	fread(&sampleRate, sizeof(DWORD), 1, f);
	fread(&avgBytesPerSec, sizeof(DWORD), 1, f);
	fread(&bytesPerSample, sizeof(short), 1, f);
	fread(&bitsPerSample, sizeof(short), 1, f);

	fread(type, sizeof(char), 4, f);
	if (type[0] != 'd' || type[1] != 'a' || type[2] != 't' || type[3] != 'a'){
		fclose(f);
		return 5;
	}

	fread(&dataSize, sizeof(DWORD), 1, f);

	unsigned char* buf = new unsigned char[dataSize];
	fread(buf, sizeof(BYTE), dataSize, f);

	ALenum format;
	if (bitsPerSample == 8){
		if (channels == 1)
			format = AL_FORMAT_MONO8;
		else if (channels == 2)
			format = AL_FORMAT_STEREO8;
	}
	else if (bitsPerSample == 16){
		if (channels == 1)
			format = AL_FORMAT_MONO16;
		else if (channels == 2)
			format = AL_FORMAT_STEREO16;
	}
	if (!format) return 6;

	sound _sound = { format, size, buf, sampleRate };
	s = _sound;

	//free(buf);

	fclose(f);
	return 0;
}

vector<string> list_audio_devices(const ALCchar *devices){
	const ALCchar *device = devices, *next = devices + 1;
	int i = 0, j = 0;
	
	vector<string> tmp;

	while (device && *device != '\0' && next && *next != '\0') {
		tmp.push_back(device);
		printf("%d. %s\n", j, device);
		i = strlen(device);
		device += (i + 1);
		next += (i + 2);
		j++;
	}

	return tmp;
}

int main(){

	vector<string> devices = list_audio_devices(alcGetString(NULL, ALC_ALL_DEVICES_SPECIFIER));
	int x = 0;
	cout << "Device ID: ";
	cin >> x;
	ALCdevice *device = alcOpenDevice(devices[x].c_str());
	if (!device)
		cout << "Error" << endl;

	ALboolean enumeration;
	enumeration = alcIsExtensionPresent(device, "ALC_ENUMERATION_EXT");
	if (enumeration == AL_FALSE)
		cout << "This device not support ALC_ENUMERATION_EXT" << endl;

	ALCcontext *context;
	context = alcCreateContext(device, NULL);
	if (!alcMakeContextCurrent(context))
		cout << "Error alcMakeContextCurrent" << endl;

	ALuint buffer, source;
	alGenBuffers(1, &buffer);
	alGenSources(1, &source);

	sound s;
	cout << load(PATH, s) << endl;

	alBufferData(buffer, s.format, s.data, s.size, s.freq);

	ALfloat SourcePos[] = { 0.0, 0.0, 0.0 };
	ALfloat SourceVel[] = { 0.0, 0.0, 0.0 };
	ALfloat ListenerPos[] = { 0.0, 0.0, 0.0 };
	ALfloat ListenerVel[] = { 0.0, 0.0, 0.0 };
	ALfloat ListenerOri[] = { 0.0, 0.0, -1.0, 0.0, 1.0, 0.0 };

	alListenerfv(AL_POSITION, ListenerPos);
	alListenerfv(AL_VELOCITY, ListenerVel);
	alListenerfv(AL_ORIENTATION, ListenerOri);

	alSourcei(source, AL_BUFFER, buffer);
	alSourcef(source, AL_PITCH, 1.0f);
	alSourcef(source, AL_GAIN, GAIN);
	alSourcefv(source, AL_POSITION, SourcePos);
	alSourcefv(source, AL_VELOCITY, SourceVel);
	alSourcei(source, AL_LOOPING, LOOP);

	bool play = false;
	while (1){
		if (GetKeyState(VK_ESCAPE) < 0)
			break;
		if (foo && play == false){
			alSourcePlay(source);
			play = true;
		}
	}

	alSourcePause(source);
	
	alDeleteSources(1, &source);
	alDeleteBuffers(1, &buffer);
	device = alcGetContextsDevice(context);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	alcCloseDevice(device);

	return 1;
}