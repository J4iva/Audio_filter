#include <iostream>
#include <fstream>
using namespace std;

struct WAV_Header{
    char riff[4]; ////Always RIFF if it's a .wav audio, we can use this to compare and let the file pass as media.
    unsigned int overall_size;
    char wave[4]; //Always WAVE if it's a .wav audio, we can use this to compare and let the file pass as audio.
    char fmt_chunk_marker[4]; //Always fmt, convention flag
    unsigned int length_fmt; //Lenght of format listed above
    unsigned short format_type; // 2bytes format description. 1 means PCM, i know there exist LPCM but don't know how to implement it at least in this way.
    unsigned short channels; //2byte, number of chanels making it mono or stereo
    unsigned int sample_rate; //4bytes always, common values are 44100 Hz used on CD.
    unsigned int byte_rate; //4bytes, (Sample Rate * BitsPerSample * Channels) / 8.
    unsigned short block_align; //2 byte, Indicates the size in bytes of a unit of sample data. 
    unsigned short bits_per_sample;
    char data_chunk_header[4]; //4 characters flag, "data"
    unsigned int data_size; //size of the next data, the sample data.
};

const int MAX_SAMPLES = 1000000;
typedef short Tsampledata[MAX_SAMPLES];

bool read(WAV_Header header, string file_name, bool open, int num_samples, Tsampledata sample_data);

struct Sample {
    double time_ms;
    short amplitude;
};

Sample samples[MAX_SAMPLES]; //array to include all and manage the program.


int main(){
    WAV_Header header;
    string file_name;
    bool open = false;
    int num_samples;
    Tsampledata sample_data; // raw amplitude values
    


    while (open == false){
    cout << "Insert the file location here: ";
    getline(cin, file_name);
    read(header, file_name, open, num_samples, sample_data);
    }

    open = false;

    num_samples = header.data_size / (header.bits_per_sample / 8);
    if (num_samples > MAX_SAMPLES){
        cout << endl << "The audio file is very long! Upload a shorter one or increase MAX_SAMPLES." << endl; //dev only
        return 0;
    }

    
    


    //output demo onñly first 100
    for (int i = 0; i < 100 && i < num_samples; i++) {  
        cout << "Time: " << samples[i].time_ms << " ms, Amplitude: " << samples[i].amplitude << endl;
    }

    return 0;


}

bool read(WAV_Header header, string file_name, bool open, int num_samples, Tsampledata sample_data){
    open = true;
    ifstream file(file_name, ios::binary);

    if (!file){
        cout << endl << "File cannot be open!!" << endl;
        open = false;
    }

    file.read(reinterpret_cast<char*>(&header), sizeof(WAV_Header)); // Making the first part of the binary fit the strcut. 

    if (header.bits_per_sample != 16 || header.channels != 1 || header.format_type != 1){
        cout << endl << "Unsupported WAV file format or not WAV file. Use .wav 16-bit mono PCM" << endl;
        open = false;
    }

    

    num_samples = header.data_size / (header.bits_per_sample / 8);
    /*if (num_samples > MAX_SAMPLES){
        cout << endl << "The audio file is very long! Upload a shorter one or increase MAX_SAMPLES." << endl; //dev only
        return 0;
    }*/

    file.read(reinterpret_cast<char*>(sample_data), num_samples * sizeof(Tsampledata)); //This part of the binary is the sample data.
    file.close();

    for (int i = 0; i < num_samples; i++){
        samples[i].amplitude = sample_data[i];
        samples[i].time_ms = (i*1000.0) / header.sample_rate;
    }  //This for loop is filling all the data in the struct "samples". We should change the 1000.0 if we want to increase the number of samples, as well as the num_samples etc.
    
    return open;
}
