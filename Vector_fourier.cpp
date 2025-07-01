#include <iostream>
#include <fstream>
#include <vector>
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

struct WAV_Sample{
    double time;
    short amplitude;
};

void read(string& file_name, int& number, WAV_Header& header, int& num_samples, vector<WAV_Sample>& sample);
int menu(string& file_name, WAV_Header& header, int& num_samples, vector<WAV_Sample>& sample);
void display(vector<WAV_Sample> sample);



int main(){
    vector<WAV_Sample> sample;
    string file_name;
    WAV_Header header;
    int num_samples;

    menu(file_name, header, num_samples, sample);

}

int menu(string& file_name, WAV_Header& header, int& num_samples, vector<WAV_Sample>& sample){
    int number = 1;

    while (number != 0){
    cout << endl << "MENU:" << endl;
    cout << "0. Exit." << endl;
    cout << "1. Insert the .wav file." << endl;
    cout << "2. Display." << endl; // for the moment a numerical display, (J4iva).
    //...
    cout << "Enter the option number: ";
    cin >> number;

    switch (number)
    {
    case 0:
        cout << "Exiting the program..." << endl;
        break;
    case 1:
        read(file_name, number, header, num_samples, sample);
        break;
    
    case 2:
        cout << endl << "HEADER INFO: " << endl;
        cout << "Sample rate: " << header.sample_rate << " Hz." << endl;
        cout << "Number of samples: " << num_samples << endl;
        cout << "Length of the audio: " << sample.back().time/1000 << " s" << endl;
        display(sample);
        break;
    case 404:
        cout << endl << "An error ocurred, going back to the menu..." << endl;
        break;
    }
  }

  return 0; //For the moment, (J4iva).

}



void read(string& file_name, int& number, WAV_Header& header, int& num_samples, vector<WAV_Sample>& sample){
    vector<short> temp_sample; //Used to read the samples directly, this could cause an error so keep track, (J4iva).
    bool okay = true;

    cout <<  endl << "Insert the file location here: ";
    cin.ignore();
    getline(cin, file_name); //Name of the file for me /home/jaiva/Downloads/01-440-sine-3s-mono.wav , (J4iva).

    ifstream file(file_name, ios::binary);

    if (!file){
        cout << endl << "File cannot be open!" << endl;
        number = 404;
        okay = false;
    }

    file.read(reinterpret_cast<char*>(&header), sizeof(WAV_Header)); // Making the first part of the binary fit the strcut, (J4iva).

    if (header.bits_per_sample != 16 || header.channels != 1 || header.format_type != 1){
        cout << endl << "Unsupported WAV file format or not WAV file. Use .wav 16-bit mono PCM" << endl;
        number = 404;
        okay = false;
    } //This could be changed to variables comparation if we create a personalize file format of the .wav file, (J4iva).


    //Second part of the read, sample part, (J4iva).
     num_samples = header.data_size / (header.bits_per_sample / 8);

     temp_sample.resize(num_samples); //making the vector equal to the number of samples so fit the next sample, (J4iva).
     file.read(reinterpret_cast<char*>(temp_sample.data()), num_samples * sizeof(short));
     file.close();

    for (int i = 0; i < num_samples; i++){
        WAV_Sample current_sample;
        current_sample.amplitude = temp_sample[i];
        current_sample.time = (i*1000)/header.sample_rate;
        sample.push_back(current_sample);
    }

    if (okay == true){
        cout << "File reading complete succesfully, going back to the menu... " << endl;
    }
    
}

void display(vector<WAV_Sample> sample){
    cout << endl << "SAMPLE DISPLAY: " << endl << endl;

    for (int i = 0; i < sample.size(); i++) {  
        cout << i << "Time: " << sample[i].time << " ms, Amplitude: " << sample[i].amplitude << endl; //Time is in ms, if want to change aksi change how time is "created" at the last for of read(), (J4iva).
    }

}